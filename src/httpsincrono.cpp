#include "httpsincrono.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

namespace
{
RespostaHttp executar(QNetworkAccessManager &gerenciador, QNetworkRequest requisicao, const QByteArray &metodo, const QByteArray &corpo)
{
    requisicao.setTransferTimeout(30000); // 30s max

    QNetworkReply *resposta = nullptr;
    if (metodo == "GET")
        resposta = gerenciador.get(requisicao);
    else if (metodo == "POST")
        resposta = gerenciador.post(requisicao, corpo);
    else if (metodo == "PATCH")
        resposta = gerenciador.sendCustomRequest(requisicao, "PATCH", corpo);

    RespostaHttp saida;
    if (!resposta) {
        saida.erro = "Método HTTP não suportado.";
        return saida;
    }

    QEventLoop loop;
    QObject::connect(resposta, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    saida.codigoStatus = resposta->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    saida.corpo = resposta->readAll();
    saida.ok = (resposta->error() == QNetworkReply::NoError) && saida.codigoStatus >= 200 && saida.codigoStatus < 300;
    if (!saida.ok && saida.erro.isEmpty())
        saida.erro = resposta->errorString();
    resposta->deleteLater();
    return saida;
}
}

namespace HttpSincrono
{
RespostaHttp get(const QString &url, const QMap<QString, QString> &cabecalhos)
{
    QNetworkAccessManager gerenciador;
    QNetworkRequest requisicao{QUrl(url)};
    for (auto it = cabecalhos.constBegin(); it != cabecalhos.constEnd(); ++it)
        requisicao.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    return executar(gerenciador, requisicao, "GET", QByteArray());
}

RespostaHttp post(const QString &url, const QByteArray &corpo, const QString &tipoConteudo, const QMap<QString, QString> &cabecalhos)
{
    QNetworkAccessManager gerenciador;
    QNetworkRequest requisicao{QUrl(url)};
    requisicao.setHeader(QNetworkRequest::ContentTypeHeader, tipoConteudo);
    for (auto it = cabecalhos.constBegin(); it != cabecalhos.constEnd(); ++it)
        requisicao.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    return executar(gerenciador, requisicao, "POST", corpo);
}

RespostaHttp postFormUrlEncoded(const QString &url, const QMap<QString, QString> &campos)
{
    QUrlQuery query;
    for (auto it = campos.constBegin(); it != campos.constEnd(); ++it)
        query.addQueryItem(it.key(), it.value());
    return post(url, query.toString(QUrl::FullyEncoded).toUtf8(), "application/x-www-form-urlencoded");
}

RespostaHttp patch(const QString &url, const QByteArray &corpo, const QString &tipoConteudo, const QMap<QString, QString> &cabecalhos)
{
    QNetworkAccessManager gerenciador;
    QNetworkRequest requisicao{QUrl(url)};
    requisicao.setHeader(QNetworkRequest::ContentTypeHeader, tipoConteudo);
    for (auto it = cabecalhos.constBegin(); it != cabecalhos.constEnd(); ++it)
        requisicao.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    return executar(gerenciador, requisicao, "PATCH", corpo);
}
}
