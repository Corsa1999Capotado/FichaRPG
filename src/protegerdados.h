#pragma once

#include <QByteArray>
#include <QString>

// Encriptação em repouso via DPAPI do Windows — atrelada à conta do usuário
// logado no Windows, sem precisar de nenhuma senha própria do app. Usado pra
// guardar o refresh_token e o Client Secret do Google Drive no disco.
namespace ProtegerDados
{
QByteArray proteger(const QString &textoClaro);
QString desproteger(const QByteArray &dadosProtegidos);
}
