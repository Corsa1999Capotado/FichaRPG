#include "googledrive.h"

#include "httpsincrono.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>

namespace GoogleDrive
{
namespace
{
QMap<QString, QString> cabecalhoAuth(const QString &accessToken)
{
    return {{"Authorization", "Bearer " + accessToken}};
}

QString escaparParaConsultaDrive(QString texto)
{
    return texto.replace("\\", "\\\\").replace("'", "\\'");
}
}

QString encontrarOuCriarPasta(const QString &accessToken, const QString &nome, const QString &idPai, QString *erroSaida)
{
    QString consulta = QString("name = '%1' and mimeType = 'application/vnd.google-apps.folder' and trashed = false")
                            .arg(escaparParaConsultaDrive(nome));
    consulta += QString(" and '%1' in parents").arg(idPai.isEmpty() ? QStringLiteral("root") : idPai);

    QUrl url("https://www.googleapis.com/drive/v3/files");
    QUrlQuery query;
    query.addQueryItem("q", consulta);
    query.addQueryItem("fields", "files(id,name)");
    query.addQueryItem("spaces", "drive");
    url.setQuery(query);

    const RespostaHttp resposta = HttpSincrono::get(url.toString(QUrl::FullyEncoded), cabecalhoAuth(accessToken));
    if (!resposta.ok) {
        if (erroSaida)
            *erroSaida = resposta.erro;
        return QString();
    }

    const QJsonArray arquivos = QJsonDocument::fromJson(resposta.corpo).object().value("files").toArray();
    if (!arquivos.isEmpty())
        return arquivos.first().toObject().value("id").toString();

    QJsonObject metadados;
    metadados["name"] = nome;
    metadados["mimeType"] = "application/vnd.google-apps.folder";
    metadados["parents"] = QJsonArray{idPai.isEmpty() ? QStringLiteral("root") : idPai};

    const RespostaHttp respostaCriacao =
        HttpSincrono::post("https://www.googleapis.com/drive/v3/files", QJsonDocument(metadados).toJson(QJsonDocument::Compact),
                            "application/json", cabecalhoAuth(accessToken));

    if (!respostaCriacao.ok) {
        if (erroSaida)
            *erroSaida = respostaCriacao.erro;
        return QString();
    }
    return QJsonDocument::fromJson(respostaCriacao.corpo).object().value("id").toString();
}

QVector<ArquivoDrive> listarArquivos(const QString &accessToken, const QString &idPasta, QString *erroSaida)
{
    QVector<ArquivoDrive> resultado;

    QUrl url("https://www.googleapis.com/drive/v3/files");
    QUrlQuery query;
    query.addQueryItem("q", QString("'%1' in parents and trashed = false").arg(idPasta));
    query.addQueryItem("fields", "files(id,name)");
    query.addQueryItem("pageSize", "1000");
    url.setQuery(query);

    const RespostaHttp resposta = HttpSincrono::get(url.toString(QUrl::FullyEncoded), cabecalhoAuth(accessToken));
    if (!resposta.ok) {
        if (erroSaida)
            *erroSaida = resposta.erro;
        return resultado;
    }

    for (const QJsonValue &v : QJsonDocument::fromJson(resposta.corpo).object().value("files").toArray()) {
        const QJsonObject o = v.toObject();
        resultado.append({o.value("id").toString(), o.value("name").toString()});
    }
    return resultado;
}

QString enviarArquivo(const QString &accessToken, const QString &idPasta, const QString &nomeArquivo, const QByteArray &conteudo,
                       const QString &mimeType, QString *erroSaida)
{
    QString erroListagem;
    const QVector<ArquivoDrive> existentes = listarArquivos(accessToken, idPasta, &erroListagem);
    for (const ArquivoDrive &arq : existentes) {
        if (arq.nome == nomeArquivo) {
            const RespostaHttp resposta =
                HttpSincrono::patch(QString("https://www.googleapis.com/upload/drive/v3/files/%1?uploadType=media").arg(arq.id),
                                     conteudo, mimeType, cabecalhoAuth(accessToken));
            if (!resposta.ok) {
                if (erroSaida)
                    *erroSaida = resposta.erro;
                return QString();
            }
            return arq.id;
        }
    }

    const QByteArray boundary = "FichaRPGBoundary_" + QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();

    QJsonObject metadados;
    metadados["name"] = nomeArquivo;
    metadados["parents"] = QJsonArray{idPasta};

    QByteArray corpo;
    corpo += "--" + boundary + "\r\n";
    corpo += "Content-Type: application/json; charset=UTF-8\r\n\r\n";
    corpo += QJsonDocument(metadados).toJson(QJsonDocument::Compact);
    corpo += "\r\n--" + boundary + "\r\n";
    corpo += "Content-Type: " + mimeType.toUtf8() + "\r\n\r\n";
    corpo += conteudo;
    corpo += "\r\n--" + boundary + "--";

    const QString tipoConteudo = QString("multipart/related; boundary=%1").arg(QString::fromUtf8(boundary));
    const RespostaHttp resposta = HttpSincrono::post("https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart", corpo,
                                                       tipoConteudo, cabecalhoAuth(accessToken));

    if (!resposta.ok) {
        if (erroSaida)
            *erroSaida = resposta.erro;
        return QString();
    }
    return QJsonDocument::fromJson(resposta.corpo).object().value("id").toString();
}

QByteArray baixarArquivo(const QString &accessToken, const QString &idArquivo, bool *okSaida)
{
    const RespostaHttp resposta =
        HttpSincrono::get(QString("https://www.googleapis.com/drive/v3/files/%1?alt=media").arg(idArquivo), cabecalhoAuth(accessToken));
    if (okSaida)
        *okSaida = resposta.ok;
    return resposta.corpo;
}
}
