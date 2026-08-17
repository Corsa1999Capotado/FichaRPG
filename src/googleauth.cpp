#include "googleauth.h"

#include "httpsincrono.h"
#include "protegerdados.h"

#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace GoogleAuth
{
namespace
{
QString caminhoConfig()
{
    const QString pasta = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(pasta);
    return pasta + "/google_drive.json";
}

QJsonObject carregarBruto()
{
    QFile arquivo(caminhoConfig());
    if (!arquivo.open(QIODevice::ReadOnly))
        return QJsonObject();
    return QJsonDocument::fromJson(arquivo.readAll()).object();
}

void salvarBruto(const QJsonObject &obj)
{
    QFile arquivo(caminhoConfig());
    if (!arquivo.open(QIODevice::WriteOnly))
        return;
    arquivo.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
}

void salvarTokens(const QString &refreshToken, const QString &email)
{
    QJsonObject raiz = carregarBruto();
    raiz["refresh_token_protegido"] = QString::fromLatin1(ProtegerDados::proteger(refreshToken).toBase64());
    raiz["user_email"] = email;
    salvarBruto(raiz);
}

QString refreshTokenSalvo()
{
    const QByteArray protegido = QByteArray::fromBase64(carregarBruto().value("refresh_token_protegido").toString().toLatin1());
    return ProtegerDados::desproteger(protegido);
}

QString gerarAleatorioBase64Url(int bytes)
{
    QByteArray dados(bytes, 0);
    for (int i = 0; i < bytes; ++i)
        dados[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    return QString::fromUtf8(dados.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}

QString desafioPkce(const QString &verificador)
{
    const QByteArray hash = QCryptographicHash::hash(verificador.toUtf8(), QCryptographicHash::Sha256);
    return QString::fromUtf8(hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}
}

Credenciais lerCredenciaisApp()
{
    const QJsonObject raiz = carregarBruto();
    Credenciais c;
    c.clientId = raiz.value("client_id").toString();
    const QByteArray protegido = QByteArray::fromBase64(raiz.value("client_secret_protegido").toString().toLatin1());
    c.clientSecret = ProtegerDados::desproteger(protegido);
    return c;
}

void salvarCredenciaisApp(const QString &clientId, const QString &clientSecret)
{
    QJsonObject raiz = carregarBruto();
    raiz["client_id"] = clientId.trimmed();
    raiz["client_secret_protegido"] = QString::fromLatin1(ProtegerDados::proteger(clientSecret.trimmed()).toBase64());
    salvarBruto(raiz);
}

bool estaConectado()
{
    return !refreshTokenSalvo().isEmpty();
}

QString emailConectado()
{
    return carregarBruto().value("user_email").toString();
}

void desconectar()
{
    QJsonObject raiz = carregarBruto();
    raiz.remove("refresh_token_protegido");
    raiz.remove("user_email");
    salvarBruto(raiz);
}

QString conectar()
{
    const Credenciais cred = lerCredenciaisApp();
    if (cred.clientId.trimmed().isEmpty() || cred.clientSecret.trimmed().isEmpty())
        return "Configure o Client ID e o Client Secret antes de conectar.";

    const QString verificador = gerarAleatorioBase64Url(64);
    const QString desafio = desafioPkce(verificador);
    const QString estado = gerarAleatorioBase64Url(16);

    QTcpServer servidor;
    if (!servidor.listen(QHostAddress::LocalHost, 0))
        return "Não foi possível abrir uma porta local pra receber o login do Google.";

    const QString redirectUri = QString("http://127.0.0.1:%1/").arg(servidor.serverPort());

    QUrl urlAuth("https://accounts.google.com/o/oauth2/v2/auth");
    QUrlQuery query;
    query.addQueryItem("client_id", cred.clientId);
    query.addQueryItem("redirect_uri", redirectUri);
    query.addQueryItem("response_type", "code");
    query.addQueryItem("scope", "https://www.googleapis.com/auth/drive.file");
    query.addQueryItem("code_challenge", desafio);
    query.addQueryItem("code_challenge_method", "S256");
    query.addQueryItem("access_type", "offline");
    query.addQueryItem("prompt", "consent");
    query.addQueryItem("state", estado);
    urlAuth.setQuery(query);

    if (!QDesktopServices::openUrl(urlAuth))
        return "Não foi possível abrir o navegador.";

    QString codigoRecebido;
    QString erroRecebido;
    bool respondeu = false;

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);

    QObject::connect(&servidor, &QTcpServer::newConnection, &loop, [&]() {
        QTcpSocket *socket = servidor.nextPendingConnection();
        if (!socket)
            return;
        QObject::connect(socket, &QTcpSocket::readyRead, &loop, [&, socket]() {
            const QByteArray dados = socket->readAll();
            const QString primeiraLinha = QString::fromUtf8(dados).split("\r\n").value(0);
            const QStringList partes = primeiraLinha.split(' ');
            if (partes.size() >= 2) {
                const QUrl url("http://127.0.0.1" + partes[1]);
                const QUrlQuery q(url);
                codigoRecebido = q.queryItemValue("code");
                erroRecebido = q.queryItemValue("error");
            }
            respondeu = true;

            const QByteArray corpoHtml = codigoRecebido.isEmpty()
                ? QByteArray("<html><body><h2>Login cancelado ou falhou. Pode fechar essa aba.</h2></body></html>")
                : QByteArray("<html><body><h2>Conectado! Pode fechar essa aba e voltar pro FichaRPG.</h2></body></html>");
            const QByteArray respostaHttp = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: "
                + QByteArray::number(corpoHtml.size()) + "\r\nConnection: close\r\n\r\n" + corpoHtml;
            socket->write(respostaHttp);
            socket->flush();
            socket->disconnectFromHost();
            loop.quit();
        });
    });

    timeoutTimer.start(120000); // 2 minutos pra completar o login no navegador
    loop.exec();

    if (!respondeu)
        return "Tempo esgotado esperando o login no navegador.";
    if (codigoRecebido.isEmpty())
        return erroRecebido.isEmpty() ? "Login cancelado." : QString("Login recusado: %1").arg(erroRecebido);

    QMap<QString, QString> campos;
    campos["client_id"] = cred.clientId;
    campos["client_secret"] = cred.clientSecret;
    campos["code"] = codigoRecebido;
    campos["code_verifier"] = verificador;
    campos["grant_type"] = "authorization_code";
    campos["redirect_uri"] = redirectUri;

    const RespostaHttp resposta = HttpSincrono::postFormUrlEncoded("https://oauth2.googleapis.com/token", campos);
    if (!resposta.ok)
        return QString("Falha ao trocar o código por tokens: %1").arg(resposta.erro);

    const QJsonObject json = QJsonDocument::fromJson(resposta.corpo).object();
    const QString refreshToken = json.value("refresh_token").toString();
    const QString accessToken = json.value("access_token").toString();
    if (refreshToken.isEmpty() || accessToken.isEmpty())
        return "O Google não devolveu os tokens esperados (confira se o Client ID/Secret estão certos).";

    QString email;
    const RespostaHttp respostaInfo =
        HttpSincrono::get("https://www.googleapis.com/oauth2/v2/userinfo", {{"Authorization", "Bearer " + accessToken}});
    if (respostaInfo.ok)
        email = QJsonDocument::fromJson(respostaInfo.corpo).object().value("email").toString();

    salvarTokens(refreshToken, email);
    return QString();
}

QString obterAccessTokenValido()
{
    const QString refreshToken = refreshTokenSalvo();
    if (refreshToken.isEmpty())
        return QString();

    const Credenciais cred = lerCredenciaisApp();
    QMap<QString, QString> campos;
    campos["client_id"] = cred.clientId;
    campos["client_secret"] = cred.clientSecret;
    campos["refresh_token"] = refreshToken;
    campos["grant_type"] = "refresh_token";

    const RespostaHttp resposta = HttpSincrono::postFormUrlEncoded("https://oauth2.googleapis.com/token", campos);
    if (!resposta.ok)
        return QString();

    return QJsonDocument::fromJson(resposta.corpo).object().value("access_token").toString();
}
}
