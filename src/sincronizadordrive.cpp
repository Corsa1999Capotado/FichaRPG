#include "sincronizadordrive.h"

#include "armazenamento.h"
#include "googleauth.h"
#include "googledrive.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>
#include <QVector>

namespace SincronizadorDrive
{
namespace
{
QString mimeParaArquivo(const QString &caminho)
{
    const QString sufixo = QFileInfo(caminho).suffix().toLower();
    if (sufixo == "json")
        return "application/json";
    if (sufixo == "png")
        return "image/png";
    if (sufixo == "jpg" || sufixo == "jpeg")
        return "image/jpeg";
    if (sufixo == "bmp")
        return "image/bmp";
    if (sufixo == "gif")
        return "image/gif";
    return "application/octet-stream";
}

struct PastasBase
{
    QString raiz, fichas, templates, imagens;
};

bool prepararPastasBase(const QString &accessToken, PastasBase &saida, QString &erro)
{
    saida.raiz = GoogleDrive::encontrarOuCriarPasta(accessToken, "FichaRPG", QString(), &erro);
    if (saida.raiz.isEmpty())
        return false;
    saida.fichas = GoogleDrive::encontrarOuCriarPasta(accessToken, "fichas", saida.raiz, &erro);
    if (saida.fichas.isEmpty())
        return false;
    saida.templates = GoogleDrive::encontrarOuCriarPasta(accessToken, "templates", saida.raiz, &erro);
    if (saida.templates.isEmpty())
        return false;
    saida.imagens = GoogleDrive::encontrarOuCriarPasta(accessToken, "imagens", saida.raiz, &erro);
    if (saida.imagens.isEmpty())
        return false;
    return true;
}
}

QString enviarTudo(QWidget *pai)
{
    const QString accessToken = GoogleAuth::obterAccessTokenValido();
    if (accessToken.isEmpty())
        return "Não foi possível autenticar com o Google. Conecte sua conta de novo.";

    PastasBase pastas;
    QString erro;
    if (!prepararPastasBase(accessToken, pastas, erro))
        return QString("Falha ao preparar as pastas no Drive: %1").arg(erro);

    struct Envio
    {
        QString caminhoLocal, nomeArquivo, idPastaDrive;
    };
    QVector<Envio> envios;

    for (const QString &categoria : Armazenamento::listarCategorias()) {
        const QString idPastaCategoria = GoogleDrive::encontrarOuCriarPasta(accessToken, categoria, pastas.fichas, &erro);
        if (idPastaCategoria.isEmpty())
            return QString("Falha ao criar a pasta \"%1\" no Drive: %2").arg(categoria, erro);
        for (const QString &caminho : Armazenamento::listarArquivosFichas(categoria))
            envios.append({caminho, QFileInfo(caminho).fileName(), idPastaCategoria});
    }
    for (const QString &caminho : Armazenamento::listarArquivosTemplates())
        envios.append({caminho, QFileInfo(caminho).fileName(), pastas.templates});

    const QDir dirImagens(Armazenamento::pastaImagens());
    for (const QString &nome : dirImagens.entryList(QDir::Files))
        envios.append({dirImagens.filePath(nome), nome, pastas.imagens});

    QProgressDialog progresso("Enviando pro Google Drive...", "Cancelar", 0, envios.size(), pai);
    progresso.setWindowModality(Qt::WindowModal);
    progresso.setMinimumDuration(0);

    int enviados = 0;
    for (const Envio &e : envios) {
        if (progresso.wasCanceled())
            return QString("Envio cancelado (%1 de %2 arquivos enviados).").arg(enviados).arg(envios.size());

        progresso.setLabelText(QString("Enviando %1...").arg(e.nomeArquivo));
        progresso.setValue(enviados);

        QFile arquivo(e.caminhoLocal);
        if (!arquivo.open(QIODevice::ReadOnly))
            continue;
        const QByteArray conteudo = arquivo.readAll();
        arquivo.close();

        QString erroEnvio;
        const QString id =
            GoogleDrive::enviarArquivo(accessToken, e.idPastaDrive, e.nomeArquivo, conteudo, mimeParaArquivo(e.caminhoLocal), &erroEnvio);
        if (id.isEmpty())
            return QString("Falha ao enviar \"%1\": %2").arg(e.nomeArquivo, erroEnvio);

        enviados++;
    }
    progresso.setValue(envios.size());
    return QString();
}

QString baixarTudo(QWidget *pai)
{
    const QString accessToken = GoogleAuth::obterAccessTokenValido();
    if (accessToken.isEmpty())
        return "Não foi possível autenticar com o Google. Conecte sua conta de novo.";

    QString erro;
    PastasBase pastas;
    if (!prepararPastasBase(accessToken, pastas, erro))
        return QString("Falha ao acessar as pastas no Drive: %1").arg(erro);

    struct Download
    {
        QString idArquivo, nomeArquivo, pastaLocalDestino;
    };
    QVector<Download> downloads;

    for (const GoogleDrive::ArquivoDrive &pastaCategoria : GoogleDrive::listarArquivos(accessToken, pastas.fichas, &erro)) {
        Armazenamento::criarCategoria(pastaCategoria.nome);
        const QString pastaLocal = Armazenamento::pastaCategoria(pastaCategoria.nome);
        for (const GoogleDrive::ArquivoDrive &arquivo : GoogleDrive::listarArquivos(accessToken, pastaCategoria.id, &erro))
            downloads.append({arquivo.id, arquivo.nome, pastaLocal});
    }
    for (const GoogleDrive::ArquivoDrive &arquivo : GoogleDrive::listarArquivos(accessToken, pastas.templates, &erro))
        downloads.append({arquivo.id, arquivo.nome, Armazenamento::pastaTemplates()});
    for (const GoogleDrive::ArquivoDrive &arquivo : GoogleDrive::listarArquivos(accessToken, pastas.imagens, &erro))
        downloads.append({arquivo.id, arquivo.nome, Armazenamento::pastaImagens()});

    QProgressDialog progresso("Baixando do Google Drive...", "Cancelar", 0, downloads.size(), pai);
    progresso.setWindowModality(Qt::WindowModal);
    progresso.setMinimumDuration(0);

    int baixados = 0;
    for (const Download &d : downloads) {
        if (progresso.wasCanceled())
            return QString("Download cancelado (%1 de %2 arquivos baixados).").arg(baixados).arg(downloads.size());

        progresso.setLabelText(QString("Baixando %1...").arg(d.nomeArquivo));
        progresso.setValue(baixados);

        bool ok = false;
        const QByteArray conteudo = GoogleDrive::baixarArquivo(accessToken, d.idArquivo, &ok);
        if (!ok)
            return QString("Falha ao baixar \"%1\".").arg(d.nomeArquivo);

        QFile destino(d.pastaLocalDestino + "/" + d.nomeArquivo);
        if (!destino.open(QIODevice::WriteOnly))
            return QString("Falha ao salvar \"%1\" localmente.").arg(d.nomeArquivo);
        destino.write(conteudo);
        destino.close();

        baixados++;
    }
    progresso.setValue(downloads.size());
    return QString();
}
}
