#include "painelfichas.h"

#include <utility>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace PainelFichas
{
namespace
{
QString caminhoConfig()
{
    const QString pasta = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(pasta);
    return pasta + "/painel_fichas.json";
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

QVector<EntradaAberta> lerEntradas(const QJsonObject &raiz)
{
    QVector<EntradaAberta> entradas;
    for (const QJsonValue &v : raiz.value("abertas").toArray()) {
        const QJsonObject o = v.toObject();
        EntradaAberta e;
        e.caminho = o.value("caminho").toString();
        e.pinada = o.value("pinada").toBool(false);
        if (!e.caminho.isEmpty() && QFileInfo::exists(e.caminho))
            entradas << e;
    }
    return entradas;
}

void escreverEntradas(QJsonObject &raiz, const QVector<EntradaAberta> &entradas)
{
    QJsonArray array;
    for (const EntradaAberta &e : entradas) {
        QJsonObject o;
        o["caminho"] = e.caminho;
        o["pinada"] = e.pinada;
        array.append(o);
    }
    raiz["abertas"] = array;
}

// posição de inserção pro topo do grupo certo (pinadas sempre acima das normais)
int inicioDoGrupo(const QVector<EntradaAberta> &entradas, bool pinada)
{
    if (pinada)
        return 0;
    int posicao = 0;
    while (posicao < entradas.size() && entradas[posicao].pinada)
        posicao++;
    return posicao;
}
}

QVector<EntradaAberta> listarAbertas()
{
    return lerEntradas(carregarBruto());
}

void registrarAcesso(const QString &caminho)
{
    QJsonObject raiz = carregarBruto();
    QVector<EntradaAberta> entradas = lerEntradas(raiz);

    int indiceExistente = -1;
    for (int i = 0; i < entradas.size(); ++i) {
        if (entradas[i].caminho == caminho) {
            indiceExistente = i;
            break;
        }
    }

    if (indiceExistente >= 0 && !reordenarAutomatico())
        return; // já está na lista e o usuário não quer reordenar sozinho

    bool pinada = false;
    if (indiceExistente >= 0) {
        pinada = entradas[indiceExistente].pinada;
        entradas.remove(indiceExistente);
    }

    EntradaAberta nova;
    nova.caminho = caminho;
    nova.pinada = pinada;
    entradas.insert(inicioDoGrupo(entradas, pinada), nova);

    const int maximo = maximoFichas();
    if (maximo > 0) {
        int naoPinadas = 0;
        for (int i = 0; i < entradas.size();) {
            if (!entradas[i].pinada) {
                naoPinadas++;
                if (naoPinadas > maximo) {
                    entradas.remove(i);
                    continue;
                }
            }
            i++;
        }
    }

    escreverEntradas(raiz, entradas);
    salvarBruto(raiz);
}

void remover(const QString &caminho)
{
    QJsonObject raiz = carregarBruto();
    QVector<EntradaAberta> entradas = lerEntradas(raiz);
    for (int i = 0; i < entradas.size(); ++i) {
        if (entradas[i].caminho == caminho) {
            entradas.remove(i);
            break;
        }
    }
    escreverEntradas(raiz, entradas);
    salvarBruto(raiz);
}

void alternarPin(const QString &caminho)
{
    QJsonObject raiz = carregarBruto();
    QVector<EntradaAberta> entradas = lerEntradas(raiz);
    for (int i = 0; i < entradas.size(); ++i) {
        if (entradas[i].caminho != caminho)
            continue;
        EntradaAberta e = entradas[i];
        e.pinada = !e.pinada;
        entradas.remove(i);
        entradas.insert(inicioDoGrupo(entradas, e.pinada), e);
        break;
    }
    escreverEntradas(raiz, entradas);
    salvarBruto(raiz);
}

void mover(const QString &caminho, int direcao)
{
    QJsonObject raiz = carregarBruto();
    QVector<EntradaAberta> entradas = lerEntradas(raiz);

    int i = -1;
    for (int k = 0; k < entradas.size(); ++k) {
        if (entradas[k].caminho == caminho) {
            i = k;
            break;
        }
    }
    if (i < 0)
        return;

    const int j = i + direcao;
    if (j < 0 || j >= entradas.size() || entradas[i].pinada != entradas[j].pinada)
        return; // só troca dentro do próprio grupo (pinada com pinada, normal com normal)

    std::swap(entradas[i], entradas[j]);
    escreverEntradas(raiz, entradas);
    salvarBruto(raiz);
}

int larguraPainel()
{
    return carregarBruto().value("larguraPainel").toInt(220);
}

void definirLarguraPainel(int largura)
{
    QJsonObject raiz = carregarBruto();
    raiz["larguraPainel"] = largura;
    salvarBruto(raiz);
}

bool mostrarAoAbrir()
{
    return carregarBruto().value("mostrarAoAbrir").toBool(false);
}

void definirMostrarAoAbrir(bool mostrar)
{
    QJsonObject raiz = carregarBruto();
    raiz["mostrarAoAbrir"] = mostrar;
    salvarBruto(raiz);
}

int maximoFichas()
{
    return carregarBruto().value("maximoFichas").toInt(10);
}

void definirMaximoFichas(int maximo)
{
    QJsonObject raiz = carregarBruto();
    raiz["maximoFichas"] = maximo;
    salvarBruto(raiz);
}

bool reordenarAutomatico()
{
    return carregarBruto().value("reordenarAutomatico").toBool(true);
}

void definirReordenarAutomatico(bool reordenar)
{
    QJsonObject raiz = carregarBruto();
    raiz["reordenarAutomatico"] = reordenar;
    salvarBruto(raiz);
}
}
