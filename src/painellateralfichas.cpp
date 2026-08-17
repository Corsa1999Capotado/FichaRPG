#include "painellateralfichas.h"

#include "armazenamento.h"
#include "cardpainel.h"
#include "character.h"
#include "gerenciadortema.h"
#include "painelfichas.h"

#include <QEasingCurve>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

PainelLateralFichas::PainelLateralFichas(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("PainelLateralFichas { background-color: #1a1a1a; border-right: 1px solid #2c2c2c; }");

    QVBoxLayout *layoutRaiz = new QVBoxLayout(this);
    layoutRaiz->setContentsMargins(12, 12, 12, 12);
    layoutRaiz->setSpacing(8);

    QHBoxLayout *linhaTitulo = new QHBoxLayout;
    QLabel *titulo = new QLabel("📋 Fichas Abertas");
    titulo->setStyleSheet("color: white; font-size: 18px; font-weight: bold; background: transparent;");
    linhaTitulo->addWidget(titulo, 1);

    QPushButton *botaoFecharPainel = new QPushButton("✕");
    botaoFecharPainel->setToolTip("Fechar painel");
    botaoFecharPainel->setFixedSize(24, 24);
    botaoFecharPainel->setCursor(Qt::PointingHandCursor);
    botaoFecharPainel->setProperty("compact", true);
    botaoFecharPainel->setStyleSheet("background-color: transparent; color: #cccccc; border: none; font-size: 14px;");
    connect(botaoFecharPainel, &QPushButton::clicked, this, &PainelLateralFichas::fechar);
    linhaTitulo->addWidget(botaoFecharPainel);

    layoutRaiz->addLayout(linhaTitulo);

    m_containerCards = new QWidget;
    m_containerCards->setStyleSheet("background: transparent;");
    m_listaLayout = new QVBoxLayout(m_containerCards);
    m_listaLayout->setAlignment(Qt::AlignTop);
    m_listaLayout->setSpacing(8);
    m_listaLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background: transparent;");
    scroll->setWidget(m_containerCards);
    layoutRaiz->addWidget(scroll, 1);

    QPushButton *botaoAdicionar = new QPushButton("➕ Adicionar ficha rápido");
    botaoAdicionar->setProperty("accent", true);
    connect(botaoAdicionar, &QPushButton::clicked, this, &PainelLateralFichas::adicionarFichaSolicitado);
    layoutRaiz->addWidget(botaoAdicionar);

    m_animacao = new QPropertyAnimation(this, "pos", this);
    m_animacao->setDuration(200);
    m_animacao->setEasingCurve(QEasingCurve::OutCubic);

    hide();
}

void PainelLateralFichas::reposicionar()
{
    if (!parentWidget())
        return;

    int largura = PainelFichas::larguraPainel();
    if (parentWidget()->width() < 1000)
        largura = qMin(largura, 180);

    setFixedWidth(largura);
    setFixedHeight(parentWidget()->height());
    move(m_aberto ? 0 : -largura, 0);
}

void PainelLateralFichas::alternarAberto()
{
    m_aberto = !m_aberto;

    if (m_aberto) {
        setFixedHeight(parentWidget() ? parentWidget()->height() : height());
        show();
        raise();
    }

    int largura = PainelFichas::larguraPainel();
    if (parentWidget() && parentWidget()->width() < 1000)
        largura = qMin(largura, 180);
    setFixedWidth(largura);

    disconnect(m_animacao, &QPropertyAnimation::finished, this, nullptr);
    m_animacao->stop();
    m_animacao->setStartValue(pos());
    m_animacao->setEndValue(QPoint(m_aberto ? 0 : -largura, 0));
    if (!m_aberto)
        connect(m_animacao, &QPropertyAnimation::finished, this, &QWidget::hide);
    m_animacao->start();
}

void PainelLateralFichas::fechar()
{
    if (m_aberto)
        alternarAberto();
}

void PainelLateralFichas::atualizarLista(const QString &caminhoAtivo)
{
    construirCards(caminhoAtivo);
}

void PainelLateralFichas::atualizarVidaFicha(const QString &caminho, int atual, int max)
{
    for (CardPainel *card : m_cards) {
        if (card->caminhoArquivo() == caminho) {
            card->atualizarVida(atual, max);
            break;
        }
    }
}

void PainelLateralFichas::construirCards(const QString &caminhoAtivo)
{
    QLayoutItem *item;
    while ((item = m_listaLayout->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    m_cards.clear();

    const QVector<PainelFichas::EntradaAberta> entradas = PainelFichas::listarAbertas();
    if (entradas.isEmpty()) {
        QLabel *vazio = new QLabel("Nenhuma ficha aberta ainda.\nClique numa ficha ou use \"+\" abaixo.");
        vazio->setWordWrap(true);
        vazio->setStyleSheet("color: #888888; font-size: 12px; background: transparent;");
        m_listaLayout->addWidget(vazio);
        return;
    }

    const QString corDestaque = GerenciadorTema::instancia().temaAtual().corAccent;

    for (int i = 0; i < entradas.size(); ++i) {
        const PainelFichas::EntradaAberta &entrada = entradas[i];
        CharacterSheet ficha;
        if (!CharacterSheet::carregarDeArquivo(entrada.caminho, ficha)) {
            PainelFichas::remover(entrada.caminho); // arquivo sumiu (ficha excluída por fora) — vale só na próxima reconstrução
            continue;
        }

        QString caminhoImagem;
        if (!ficha.imagemArquivo.isEmpty())
            caminhoImagem = Armazenamento::pastaImagens() + "/" + ficha.imagemArquivo;

        CardPainel *card = new CardPainel(entrada.caminho, ficha.nome, caminhoImagem, ficha.vidaAtual, ficha.vidaMax,
                                           entrada.pinada, corDestaque, QPointF(ficha.imagemFocoX, ficha.imagemFocoY));
        card->definirAtiva(entrada.caminho == caminhoAtivo);
        card->definirPodeSubir(i > 0 && entradas[i - 1].pinada == entrada.pinada);
        card->definirPodeDescer(i < entradas.size() - 1 && entradas[i + 1].pinada == entrada.pinada);

        connect(card, &CardPainel::ativado, this, &PainelLateralFichas::fichaSelecionada);
        connect(card, &CardPainel::fechado, this, [this, caminhoAtivo](const QString &caminho) {
            PainelFichas::remover(caminho);
            atualizarLista(caminhoAtivo);
        });
        connect(card, &CardPainel::moverCima, this, [this, caminhoAtivo](const QString &caminho) {
            PainelFichas::mover(caminho, -1);
            atualizarLista(caminhoAtivo);
        });
        connect(card, &CardPainel::moverBaixo, this, [this, caminhoAtivo](const QString &caminho) {
            PainelFichas::mover(caminho, 1);
            atualizarLista(caminhoAtivo);
        });
        connect(card, &CardPainel::pinAlternado, this, [this, caminhoAtivo](const QString &caminho) {
            PainelFichas::alternarPin(caminho);
            atualizarLista(caminhoAtivo);
        });

        m_listaLayout->addWidget(card);
        m_cards.append(card);
    }
}
