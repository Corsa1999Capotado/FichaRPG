#include "cardpainel.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

CardPainel::CardPainel(const QString &caminhoArquivo, const QString &nome, const QString &caminhoImagem,
                        int vidaAtual, int vidaMax, bool pinada, const QString &corDestaque, QWidget *parent)
    : QWidget(parent)
    , m_caminhoArquivo(caminhoArquivo)
    , m_corDestaque(corDestaque)
{
    setFixedSize(195, 120);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);
    aplicarEstiloBase();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    QLabel *fotoLabel = new QLabel;
    fotoLabel->setFixedSize(90, 90);
    fotoLabel->setAlignment(Qt::AlignCenter);
    fotoLabel->setStyleSheet("background-color: #333333; border-radius: 8px; color: #999999; font-size: 10px;");
    QPixmap pixmap;
    if (!caminhoImagem.isEmpty())
        pixmap.load(caminhoImagem);
    if (pixmap.isNull())
        fotoLabel->setText("Sem foto");
    else
        fotoLabel->setPixmap(pixmap.scaled(fotoLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    layout->addWidget(fotoLabel);

    QVBoxLayout *colunaInfo = new QVBoxLayout;
    colunaInfo->setSpacing(4);

    QLabel *nomeLabel = new QLabel(nome.isEmpty() ? "Sem nome" : nome);
    nomeLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold; background: transparent;");
    nomeLabel->setWordWrap(true);
    colunaInfo->addWidget(nomeLabel);

    m_vidaLabel = new QLabel;
    m_vidaLabel->setStyleSheet("color: #ff5c5c; font-size: 12px; font-weight: bold; background: transparent;");
    colunaInfo->addWidget(m_vidaLabel);
    atualizarVida(vidaAtual, vidaMax);

    if (pinada) {
        QLabel *pinLabel = new QLabel("📌 Fixada");
        pinLabel->setStyleSheet("color: #999999; font-size: 10px; background: transparent;");
        colunaInfo->addWidget(pinLabel);
    }

    colunaInfo->addStretch();
    layout->addLayout(colunaInfo, 1);

    // Botões de ação flutuantes, só aparecem no hover (ver enterEvent/leaveEvent).
    m_botaoFechar = new QPushButton("✕", this);
    m_botaoFechar->setToolTip("Remover do painel");
    m_botaoCima = new QPushButton("▲", this);
    m_botaoCima->setToolTip("Mover pra cima");
    m_botaoBaixo = new QPushButton("▼", this);
    m_botaoBaixo->setToolTip("Mover pra baixo");
    m_botaoPin = new QPushButton(pinada ? "📌" : "📍", this);
    m_botaoPin->setToolTip(pinada ? "Desafixar" : "Fixar no topo");

    for (QPushButton *b : {m_botaoFechar, m_botaoCima, m_botaoBaixo, m_botaoPin}) {
        b->setFixedSize(18, 18);
        b->setProperty("compact", true);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet("background-color: rgba(0,0,0,150); color: white; border: none; border-radius: 3px; font-size: 10px;");
        b->hide();
        b->raise();
    }
    m_botaoFechar->move(width() - m_botaoFechar->width() - 4, 4);
    m_botaoPin->move(width() - m_botaoPin->width() - 4, 24);
    m_botaoCima->move(width() - m_botaoCima->width() - 4, height() - 40);
    m_botaoBaixo->move(width() - m_botaoBaixo->width() - 4, height() - 20);

    connect(m_botaoFechar, &QPushButton::clicked, this, [this]() { emit fechado(m_caminhoArquivo); });
    connect(m_botaoCima, &QPushButton::clicked, this, [this]() { emit moverCima(m_caminhoArquivo); });
    connect(m_botaoBaixo, &QPushButton::clicked, this, [this]() { emit moverBaixo(m_caminhoArquivo); });
    connect(m_botaoPin, &QPushButton::clicked, this, [this]() { emit pinAlternado(m_caminhoArquivo); });
}

void CardPainel::aplicarEstiloBase()
{
    setStyleSheet("CardPainel { background-color: #242424; border: 1px solid #333333; border-radius: 8px; }"
                   "CardPainel:hover { background-color: #2a2a2a; border: 1px solid #4a4a4a; }");
}

void CardPainel::definirAtiva(bool ativa)
{
    if (!ativa) {
        aplicarEstiloBase();
        return;
    }
    setStyleSheet(QString("CardPainel { background-color: #242424; border: 2px solid %1; border-radius: 8px; }"
                           "CardPainel:hover { background-color: #2a2a2a; border: 2px solid %1; }")
                      .arg(m_corDestaque));
}

void CardPainel::atualizarVida(int atual, int max)
{
    m_vidaLabel->setText(max > 0 ? QString("❤️ %1/%2").arg(atual).arg(max) : QString());
}

void CardPainel::definirPodeSubir(bool pode)
{
    m_botaoCima->setEnabled(pode);
}

void CardPainel::definirPodeDescer(bool pode)
{
    m_botaoBaixo->setEnabled(pode);
}

void CardPainel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && rect().contains(event->pos()))
        emit ativado(m_caminhoArquivo);
    QWidget::mouseReleaseEvent(event);
}

void CardPainel::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    for (QPushButton *b : {m_botaoFechar, m_botaoCima, m_botaoBaixo, m_botaoPin})
        b->show();
}

void CardPainel::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    for (QPushButton *b : {m_botaoFechar, m_botaoCima, m_botaoBaixo, m_botaoPin})
        b->hide();
}
