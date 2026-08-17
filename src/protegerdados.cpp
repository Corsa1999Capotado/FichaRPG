#include "protegerdados.h"

#include <windows.h>

#include <wincrypt.h>

namespace ProtegerDados
{
QByteArray proteger(const QString &textoClaro)
{
    if (textoClaro.isEmpty())
        return QByteArray();

    const QByteArray utf8 = textoClaro.toUtf8();

    DATA_BLOB entrada;
    entrada.pbData = reinterpret_cast<BYTE *>(const_cast<char *>(utf8.constData()));
    entrada.cbData = static_cast<DWORD>(utf8.size());

    DATA_BLOB saida;
    saida.pbData = nullptr;
    saida.cbData = 0;
    if (!CryptProtectData(&entrada, L"FichaRPG", nullptr, nullptr, nullptr, 0, &saida))
        return QByteArray();

    const QByteArray resultado(reinterpret_cast<const char *>(saida.pbData), static_cast<int>(saida.cbData));
    LocalFree(saida.pbData);
    return resultado;
}

QString desproteger(const QByteArray &dadosProtegidos)
{
    if (dadosProtegidos.isEmpty())
        return QString();

    DATA_BLOB entrada;
    entrada.pbData = reinterpret_cast<BYTE *>(const_cast<char *>(dadosProtegidos.constData()));
    entrada.cbData = static_cast<DWORD>(dadosProtegidos.size());

    DATA_BLOB saida;
    saida.pbData = nullptr;
    saida.cbData = 0;
    if (!CryptUnprotectData(&entrada, nullptr, nullptr, nullptr, nullptr, 0, &saida))
        return QString();

    const QString resultado = QString::fromUtf8(reinterpret_cast<const char *>(saida.pbData), static_cast<int>(saida.cbData));
    LocalFree(saida.pbData);
    return resultado;
}
}
