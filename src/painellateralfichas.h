#pragma once

#include <QVector>
#include <QWidget>

class QVBoxLayout;
class QPropertyAnimation;
class CardPainel;

// Painel Lateral de Fichas Rápidas: lista deslizante (slide-in/out) de fichas
// recém-abertas/fixadas, ancorada à esquerda da tela de visualização, pra
// consultar ou trocar de ficha sem sair dela. Fica sempre em tema escuro fixo
// (é um painel utilitário, não segue o tema claro/escuro do app).
class PainelLateralFichas : public QWidget
{
    Q_OBJECT

public:
    explicit PainelLateralFichas(QWidget *parent = nullptr);

    void alternarAberto();
    bool estaAberto() const { return m_aberto; }
    void fechar();

    void atualizarLista(const QString &caminhoAtivo);
    void atualizarVidaFicha(const QString &caminho, int atual, int max);

    // Reposiciona (ancorado à esquerda, altura total do pai) — chamar no resizeEvent do pai.
    void reposicionar();

signals:
    void fichaSelecionada(const QString &caminhoArquivo);
    void adicionarFichaSolicitado();

private:
    void construirCards(const QString &caminhoAtivo);

    QWidget *m_containerCards;
    QVBoxLayout *m_listaLayout;
    QPropertyAnimation *m_animacao;
    bool m_aberto = false;
    QVector<CardPainel *> m_cards;
};
