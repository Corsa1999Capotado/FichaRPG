#pragma once

#include <QString>

// Login OAuth2 (PKCE, redirect loopback local) com o Google, escopo
// drive.file (só a pasta que o app cria, não o Drive inteiro). Guarda o
// Client ID/Secret e o refresh_token localmente, protegidos por DPAPI.
namespace GoogleAuth
{
struct Credenciais
{
    QString clientId;
    QString clientSecret;
};

Credenciais lerCredenciaisApp();
void salvarCredenciaisApp(const QString &clientId, const QString &clientSecret);

bool estaConectado();
QString emailConectado();

// Abre o navegador, escuta localhost, troca o código pelos tokens. Bloqueia
// (com timeout de 2min) até o usuário autorizar/cancelar. Devolve mensagem de
// erro, ou string vazia em caso de sucesso.
QString conectar();

void desconectar();

// Garante um access_token válido (troca o refresh_token salvo por um novo).
// Devolve string vazia se não conseguir (ex: não conectado, refresh_token revogado).
QString obterAccessTokenValido();
}
