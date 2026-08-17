#pragma once

#include <QByteArray>
#include <QMap>
#include <QString>

// Wrapper fino sobre QNetworkAccessManager que espera a resposta terminar
// antes de devolver (via QEventLoop local — a UI continua repintando durante
// a espera, só essa chamada específica fica "sequencial"). Usado pelo login
// do Google e pela sincronização manual com o Drive (Fase 1), onde um fluxo
// simples de seguir vale mais que paralelismo real.
struct RespostaHttp
{
    bool ok = false;
    int codigoStatus = 0;
    QByteArray corpo;
    QString erro;
};

namespace HttpSincrono
{
RespostaHttp get(const QString &url, const QMap<QString, QString> &cabecalhos = {});
RespostaHttp post(const QString &url, const QByteArray &corpo, const QString &tipoConteudo, const QMap<QString, QString> &cabecalhos = {});
RespostaHttp postFormUrlEncoded(const QString &url, const QMap<QString, QString> &campos);
RespostaHttp patch(const QString &url, const QByteArray &corpo, const QString &tipoConteudo, const QMap<QString, QString> &cabecalhos = {});
}
