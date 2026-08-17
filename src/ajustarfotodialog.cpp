#include "ajustarfotodialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
constexpr int kLadoVisor = 360;
}

VisorArrastoFoto::VisorArrastoFoto(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(kLadoVisor, kLadoVisor);
    setCursor(Qt::OpenHandCursor);
}

void VisorArrastoFoto::recalcularEscalada()
{
    if (m_origem.isNull())
        return;

    const qreal escala =
        std::max(qreal(width()) / m_origem.width(), qreal(height()) / m_origem.height());
    const QSize tamanho(std::max(1, int(std::ceil(m_origem.width() * escala))),
                         std::max(1, int(std::ceil(m_origem.height() * escala))));
    m_escalada = m_origem.scaled(tamanho, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void VisorArrastoFoto::definirImagem(const QPixmap &origem, QPointF focoInicial)
{
    m_origem = origem;
    recalcularEscalada();

    if (m_escalada.isNull()) {
        m_offset = QPointF(0, 0);
        update();
        return;
    }

    const qreal x = std::clamp(focoInicial.x(), 0.0, 1.0) * m_escalada.width() - width() / 2.0;
    const qreal y = std::clamp(focoInicial.y(), 0.0, 1.0) * m_escalada.height() - height() / 2.0;
    aplicarOffset(QPointF(x, y));
}

void VisorArrastoFoto::aplicarOffset(QPointF offset)
{
    if (m_escalada.isNull())
        return;

    const qreal maxX = std::max(0, m_escalada.width() - width());
    const qreal maxY = std::max(0, m_escalada.height() - height());
    m_offset.setX(std::clamp(offset.x(), 0.0, maxX));
    m_offset.setY(std::clamp(offset.y(), 0.0, maxY));
    update();
}

QPointF VisorArrastoFoto::foco() const
{
    if (m_escalada.isNull())
        return QPointF(0.5, 0.5);

    const qreal fx = (m_offset.x() + width() / 2.0) / m_escalada.width();
    const qreal fy = (m_offset.y() + height() / 2.0) / m_escalada.height();
    return QPointF(std::clamp(fx, 0.0, 1.0), std::clamp(fy, 0.0, 1.0));
}

void VisorArrastoFoto::redefinirParaCentro()
{
    if (m_escalada.isNull())
        return;
    aplicarOffset(QPointF((m_escalada.width() - width()) / 2.0, (m_escalada.height() - height()) / 2.0));
}

void VisorArrastoFoto::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    if (!m_escalada.isNull())
        painter.drawPixmap(0, 0, m_escalada, m_offset.x(), m_offset.y(), width(), height());
}

void VisorArrastoFoto::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    const QPointF focoAtual = foco();
    recalcularEscalada();
    definirImagem(m_origem, focoAtual);
}

void VisorArrastoFoto::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;
    m_arrastando = true;
    m_posInicioArrasto = event->pos();
    m_offsetInicioArrasto = m_offset;
    setCursor(Qt::ClosedHandCursor);
}

void VisorArrastoFoto::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_arrastando)
        return;
    const QPoint delta = event->pos() - m_posInicioArrasto;
    aplicarOffset(m_offsetInicioArrasto - QPointF(delta.x(), delta.y()));
}

void VisorArrastoFoto::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_arrastando = false;
    setCursor(Qt::OpenHandCursor);
}

AjustarFotoDialog::AjustarFotoDialog(const QString &caminhoImagem, QPointF focoInicial, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Ajustar enquadramento");

    QVBoxLayout *layoutRaiz = new QVBoxLayout(this);

    QLabel *instrucao = new QLabel("Arraste a imagem pra escolher qual parte fica visível na miniatura.");
    instrucao->setWordWrap(true);
    layoutRaiz->addWidget(instrucao);

    m_visor = new VisorArrastoFoto(this);
    m_visor->definirImagem(QPixmap(caminhoImagem), focoInicial);
    QHBoxLayout *linhaVisor = new QHBoxLayout;
    linhaVisor->addStretch();
    linhaVisor->addWidget(m_visor);
    linhaVisor->addStretch();
    layoutRaiz->addLayout(linhaVisor);

    QPushButton *botaoRedefinir = new QPushButton("Redefinir (centro)");
    connect(botaoRedefinir, &QPushButton::clicked, m_visor, &VisorArrastoFoto::redefinirParaCentro);
    layoutRaiz->addWidget(botaoRedefinir);

    QHBoxLayout *linhaBotoes = new QHBoxLayout;
    QPushButton *botaoCancelar = new QPushButton("Cancelar");
    connect(botaoCancelar, &QPushButton::clicked, this, &QDialog::reject);
    QPushButton *botaoOk = new QPushButton("OK");
    botaoOk->setProperty("accent", true);
    connect(botaoOk, &QPushButton::clicked, this, &QDialog::accept);
    linhaBotoes->addWidget(botaoCancelar);
    linhaBotoes->addWidget(botaoOk);
    layoutRaiz->addLayout(linhaBotoes);
}

QPointF AjustarFotoDialog::foco() const
{
    return m_visor->foco();
}
