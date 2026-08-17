#pragma once

#include <QString>
#include <QVector>

// Chamadas REST da API do Google Drive v3 — só o necessário pra sincronização
// manual da Fase 1: achar/criar pastas, listar, enviar e baixar arquivos.
namespace GoogleDrive
{
struct ArquivoDrive
{
    QString id;
    QString nome;
};

// Acha uma pasta pelo nome dentro do pai (idPai vazio = raiz "Meu Drive"); cria
// se não existir. Devolve o ID da pasta, ou vazio em caso de erro.
QString encontrarOuCriarPasta(const QString &accessToken, const QString &nome, const QString &idPai, QString *erroSaida = nullptr);

QVector<ArquivoDrive> listarArquivos(const QString &accessToken, const QString &idPasta, QString *erroSaida = nullptr);

// Cria um arquivo novo dentro da pasta e devolve o ID. Se já existir um
// arquivo com esse nome ali, atualiza o conteúdo dele em vez de duplicar.
QString enviarArquivo(const QString &accessToken, const QString &idPasta, const QString &nomeArquivo, const QByteArray &conteudo,
                       const QString &mimeType, QString *erroSaida = nullptr);

QByteArray baixarArquivo(const QString &accessToken, const QString &idArquivo, bool *okSaida = nullptr);
}
