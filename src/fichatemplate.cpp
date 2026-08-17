#include "fichatemplate.h"

#include <QFile>
#include <QJsonDocument>

QJsonObject FichaTemplate::toJson() const
{
    QJsonObject obj;
    obj["nomeTemplate"] = nomeTemplate;
    obj["ficha"] = base.toJson();
    return obj;
}

FichaTemplate FichaTemplate::fromJson(const QJsonObject &obj)
{
    FichaTemplate t;
    t.nomeTemplate = obj.value("nomeTemplate").toString();
    t.base = CharacterSheet::fromJson(obj.value("ficha").toObject());
    return t;
}

bool FichaTemplate::salvarEmArquivo(const QString &caminho) const
{
    QFile arquivo(caminho);
    if (!arquivo.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    const QJsonDocument doc(toJson());
    arquivo.write(doc.toJson(QJsonDocument::Indented));
    arquivo.close();
    return true;
}

bool FichaTemplate::carregarDeArquivo(const QString &caminho, FichaTemplate &destino)
{
    QFile arquivo(caminho);
    if (!arquivo.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    const QByteArray dados = arquivo.readAll();
    arquivo.close();

    QJsonParseError erro;
    const QJsonDocument doc = QJsonDocument::fromJson(dados, &erro);
    if (erro.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    destino = FichaTemplate::fromJson(doc.object());
    return true;
}
