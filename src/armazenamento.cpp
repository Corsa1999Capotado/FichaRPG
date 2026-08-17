#include "armazenamento.h"

#include <algorithm>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUuid>

namespace Armazenamento
{

static QString pastaBase()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

// Guarda ordem/ícone das pastas (coisas que não dá pra representar só com o
// nome do diretório físico). Chave = nome da categoria.
static QString caminhoMetaPastas()
{
    return pastaBase() + "/pastas_meta.json";
}

static QJsonObject carregarMetaPastas()
{
    QFile arquivo(caminhoMetaPastas());
    if (!arquivo.open(QIODevice::ReadOnly))
        return QJsonObject();
    return QJsonDocument::fromJson(arquivo.readAll()).object();
}

static void salvarMetaPastas(const QJsonObject &meta)
{
    QFile arquivo(caminhoMetaPastas());
    if (!arquivo.open(QIODevice::WriteOnly))
        return;
    arquivo.write(QJsonDocument(meta).toJson(QJsonDocument::Compact));
}

QString pastaFichas()
{
    const QString caminho = pastaBase() + "/fichas";
    QDir().mkpath(caminho);
    return caminho;
}

QString pastaCategoria(const QString &categoria)
{
    const QString caminho = pastaFichas() + "/" + categoria;
    QDir().mkpath(caminho);
    return caminho;
}

QString pastaImagens()
{
    const QString caminho = pastaBase() + "/imagens";
    QDir().mkpath(caminho);
    return caminho;
}

QString pastaTemplates()
{
    const QString caminho = pastaBase() + "/templates";
    QDir().mkpath(caminho);
    return caminho;
}

static bool nomeReservado(const QString &nome)
{
    // "imagens" é o nome da pasta compartilhada de fotos (pastaImagens()) — não
    // pode virar categoria, senão as fichas somem visualmente de dentro dela.
    return nome.compare("imagens", Qt::CaseInsensitive) == 0;
}

bool criarCategoria(const QString &nome, const QString &icone)
{
    const QString nomeLimpo = nome.trimmed();
    if (nomeLimpo.isEmpty() || nomeReservado(nomeLimpo))
        return false;
    if (!QDir().mkpath(pastaCategoria(nomeLimpo)))
        return false;

    QJsonObject meta = carregarMetaPastas();
    int maiorOrdem = -1;
    for (const QString &chave : meta.keys())
        maiorOrdem = qMax(maiorOrdem, meta.value(chave).toObject().value("ordem").toInt());

    QJsonObject entrada;
    entrada["icone"] = icone;
    entrada["ordem"] = maiorOrdem + 1;
    meta[nomeLimpo] = entrada;
    salvarMetaPastas(meta);
    return true;
}

QStringList listarCategorias()
{
    const QDir dir(pastaFichas());
    const QStringList todas = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    QStringList categorias;
    for (const QString &c : todas) {
        if (!nomeReservado(c))
            categorias << c;
    }

    if (categorias.isEmpty()) {
        criarCategoria("Personagens", "👥");
        categorias << "Personagens";
    }

    const QJsonObject meta = carregarMetaPastas();
    std::sort(categorias.begin(), categorias.end(), [&meta](const QString &a, const QString &b) {
        const int ordemA = meta.value(a).toObject().value("ordem").toInt(1000000);
        const int ordemB = meta.value(b).toObject().value("ordem").toInt(1000000);
        if (ordemA != ordemB)
            return ordemA < ordemB;
        return a.localeAwareCompare(b) < 0;
    });
    return categorias;
}

bool excluirCategoria(const QString &categoria)
{
    if (nomeReservado(categoria))
        return false;
    QDir dir(pastaCategoria(categoria));
    const bool removeu = dir.removeRecursively();
    if (removeu) {
        QJsonObject meta = carregarMetaPastas();
        if (meta.contains(categoria)) {
            meta.remove(categoria);
            salvarMetaPastas(meta);
        }
    }
    return removeu;
}

bool renomearCategoria(const QString &nomeAtual, const QString &novoNome)
{
    const QString limpo = novoNome.trimmed();
    if (limpo.isEmpty() || nomeReservado(limpo) || limpo == nomeAtual)
        return false;
    if (QFileInfo::exists(pastaFichas() + "/" + limpo))
        return false;

    QDir dir(pastaFichas());
    if (!dir.rename(nomeAtual, limpo))
        return false;

    QJsonObject meta = carregarMetaPastas();
    if (meta.contains(nomeAtual)) {
        meta[limpo] = meta.value(nomeAtual);
        meta.remove(nomeAtual);
        salvarMetaPastas(meta);
    }
    return true;
}

void reordenarCategorias(const QStringList &novaOrdem)
{
    QJsonObject meta = carregarMetaPastas();
    for (int i = 0; i < novaOrdem.size(); ++i) {
        QJsonObject entrada = meta.value(novaOrdem[i]).toObject();
        entrada["ordem"] = i;
        meta[novaOrdem[i]] = entrada;
    }
    salvarMetaPastas(meta);
}

QString iconeCategoria(const QString &categoria)
{
    return carregarMetaPastas().value(categoria).toObject().value("icone").toString();
}

bool definirIconeCategoria(const QString &categoria, const QString &icone)
{
    QJsonObject meta = carregarMetaPastas();
    QJsonObject entrada = meta.value(categoria).toObject();
    entrada["icone"] = icone;
    if (!entrada.contains("ordem"))
        entrada["ordem"] = meta.size();
    meta[categoria] = entrada;
    salvarMetaPastas(meta);
    return true;
}

bool moverTodasFichas(const QString &categoriaOrigem, const QString &categoriaDestino)
{
    const QStringList arquivos = listarArquivosFichas(categoriaOrigem);
    const QString pastaDestino = pastaCategoria(categoriaDestino);

    bool tudoOk = true;
    for (const QString &caminho : arquivos) {
        const QFileInfo info(caminho);
        QString destino = pastaDestino + "/" + info.fileName();
        int contador = 1;
        while (QFileInfo::exists(destino)) {
            destino = QString("%1/%2_%3.json").arg(pastaDestino, info.completeBaseName()).arg(contador);
            contador++;
        }
        if (!QFile::rename(caminho, destino))
            tudoOk = false;
    }
    return tudoOk;
}

QStringList listarArquivosFichas(const QString &categoria)
{
    const QDir dir(pastaCategoria(categoria));
    const QStringList nomes = dir.entryList(QStringList() << "*.json", QDir::Files, QDir::Name);

    QStringList caminhos;
    for (const QString &nome : nomes)
        caminhos << dir.filePath(nome);
    return caminhos;
}

QStringList listarArquivosTemplates()
{
    const QDir dir(pastaTemplates());
    const QStringList nomes = dir.entryList(QStringList() << "*.json", QDir::Files, QDir::Name);

    QStringList caminhos;
    for (const QString &nome : nomes)
        caminhos << dir.filePath(nome);
    return caminhos;
}

static QString paraSlug(const QString &texto, const QString &padrao)
{
    QString slug = texto.trimmed();
    slug.replace(QRegularExpression("[^A-Za-z0-9À-ÿ_-]+"), "_");
    if (slug.isEmpty())
        slug = padrao;
    return slug;
}

QString gerarNomeArquivoUnico(const QString &categoria, const QString &baseNome)
{
    const QString slug = paraSlug(baseNome, "ficha");
    const QString pasta = pastaCategoria(categoria);

    QString caminho = pasta + "/" + slug + ".json";
    int contador = 1;
    while (QFileInfo::exists(caminho)) {
        caminho = QString("%1/%2_%3.json").arg(pasta, slug).arg(contador);
        contador++;
    }
    return caminho;
}

QString gerarNomeArquivoTemplateUnico(const QString &baseNome)
{
    const QString slug = paraSlug(baseNome, "template");
    const QString pasta = pastaTemplates();

    QString caminho = pasta + "/" + slug + ".json";
    int contador = 1;
    while (QFileInfo::exists(caminho)) {
        caminho = QString("%1/%2_%3.json").arg(pasta, slug).arg(contador);
        contador++;
    }
    return caminho;
}

QString copiarImagemParaPasta(const QString &caminhoOrigem, const QString &nomeBase)
{
    if (caminhoOrigem.isEmpty())
        return QString();

    const QFileInfo origem(caminhoOrigem);
    const QString slug = paraSlug(nomeBase, "personagem");
    const QString destino = pastaImagens() + "/" + slug + "_" + QUuid::createUuid().toString(QUuid::WithoutBraces) + "." + origem.suffix();

    if (QFile::copy(caminhoOrigem, destino))
        return QFileInfo(destino).fileName();
    return QString();
}

static QString pastaBackups(const QString &caminhoFicha)
{
    const QFileInfo info(caminhoFicha);
    const QString caminho = info.absolutePath() + "/.backups/" + info.completeBaseName();
    QDir().mkpath(caminho);
    return caminho;
}

bool salvarBackup(const QString &caminhoFicha)
{
    QFile origem(caminhoFicha);
    if (!origem.exists() || !origem.open(QIODevice::ReadOnly))
        return false;

    const QByteArray dados = origem.readAll();
    origem.close();

    const QString pasta = pastaBackups(caminhoFicha);
    const QString carimbo = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");

    QFile destino(pasta + "/" + carimbo + ".bak");
    if (!destino.open(QIODevice::WriteOnly))
        return false;
    destino.write(qCompress(dados));
    destino.close();

    // mantém só os 5 backups mais recentes
    QDir dir(pasta);
    QStringList arquivos = dir.entryList(QStringList() << "*.bak", QDir::Files, QDir::Name);
    while (arquivos.size() > 5) {
        QFile::remove(dir.filePath(arquivos.first()));
        arquivos.removeFirst();
    }

    return true;
}

QStringList listarBackups(const QString &caminhoFicha)
{
    const QDir dir(pastaBackups(caminhoFicha));
    const QStringList nomes = dir.entryList(QStringList() << "*.bak", QDir::Files, QDir::Name);

    QStringList caminhos;
    for (int i = nomes.size() - 1; i >= 0; --i)
        caminhos << dir.filePath(nomes[i]);
    return caminhos;
}

bool restaurarBackup(const QString &caminhoBackup, QByteArray &jsonDestino)
{
    QFile arquivo(caminhoBackup);
    if (!arquivo.open(QIODevice::ReadOnly))
        return false;

    const QByteArray comprimido = arquivo.readAll();
    arquivo.close();

    jsonDestino = qUncompress(comprimido);
    return !jsonDestino.isEmpty();
}

bool excluirFicha(const QString &caminhoFicha)
{
    const bool removeu = QFile::remove(caminhoFicha);
    QDir(pastaBackups(caminhoFicha)).removeRecursively(); // apaga o histórico de backups junto
    return removeu;
}

bool excluirTemplate(const QString &caminhoTemplate)
{
    return QFile::remove(caminhoTemplate);
}

}
