#pragma once

#include <QString>
#include <QVector>

// Dados e preferências do Painel Lateral de Fichas Rápidas: quais fichas estão
// "abertas" (recém-acessadas ou fixadas) e como o painel deve se comportar.
// Persistido num JSON próprio na pasta de dados do usuário.
namespace PainelFichas
{
struct EntradaAberta
{
    QString caminho;
    bool pinada = false;
};

QVector<EntradaAberta> listarAbertas();

// Registra que uma ficha foi acessada: se reordenarAutomatico() estiver ligado,
// manda ela pro topo do seu grupo (pinadas sempre acima das não-pinadas); se
// ainda não estava na lista, insere no topo das não-pinadas, cortando o excesso
// conforme maximoFichas().
void registrarAcesso(const QString &caminho);

void remover(const QString &caminho);
void alternarPin(const QString &caminho);
void mover(const QString &caminho, int direcao); // -1 sobe, +1 desce (dentro do próprio grupo)

int larguraPainel(); // padrão 220
void definirLarguraPainel(int largura);

bool mostrarAoAbrir(); // padrão false
void definirMostrarAoAbrir(bool mostrar);

int maximoFichas(); // padrão 10; 0 = sem limite
void definirMaximoFichas(int maximo);

bool reordenarAutomatico(); // padrão true
void definirReordenarAutomatico(bool reordenar);
}
