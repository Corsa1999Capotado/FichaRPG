#include "preferencias.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace Preferencias
{
namespace
{
QString caminhoConfig()
{
    const QString pasta = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(pasta);
    return pasta + "/preferencias.json";
}

QJsonObject carregar()
{
    QFile arquivo(caminhoConfig());
    if (!arquivo.open(QIODevice::ReadOnly))
        return QJsonObject();
    return QJsonDocument::fromJson(arquivo.readAll()).object();
}

void salvar(const QJsonObject &obj)
{
    QFile arquivo(caminhoConfig());
    if (!arquivo.open(QIODevice::WriteOnly))
        return;
    arquivo.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
}
}

int colunasGrid()
{
    const int colunas = carregar().value("colunasGrid").toInt(3);
    return (colunas == 2 || colunas == 4) ? colunas : 3;
}

void definirColunasGrid(int colunas)
{
    QJsonObject obj = carregar();
    obj["colunasGrid"] = colunas;
    salvar(obj);
}

bool confirmarAntesDeExcluir()
{
    return carregar().value("confirmarAntesDeExcluir").toBool(true);
}

void definirConfirmarAntesDeExcluir(bool confirmar)
{
    QJsonObject obj = carregar();
    obj["confirmarAntesDeExcluir"] = confirmar;
    salvar(obj);
}
}
