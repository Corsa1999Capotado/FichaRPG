#pragma once

#include <QDialog>
#include <QPixmap>
#include <QPointF>
#include <QWidget>

// Área quadrada de arrasto: mostra a imagem "cobrindo" o quadro (igual à
// miniatura final) e deixa o usuário arrastá-la com o mouse pra escolher
// qual parte fica visível.
class VisorArrastoFoto : public QWidget
{
    Q_OBJECT

public:
    explicit VisorArrastoFoto(QWidget *parent = nullptr);

    void definirImagem(const QPixmap &origem, QPointF focoInicial);
    QPointF foco() const;
    void redefinirParaCentro();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void recalcularEscalada();
    void aplicarOffset(QPointF offset);

    QPixmap m_origem;
    QPixmap m_escalada;
    QPointF m_offset;         // topo-esquerdo visível, em pixels da imagem escalada
    QPoint m_posInicioArrasto;
    QPointF m_offsetInicioArrasto;
    bool m_arrastando = false;
};

// Diálogo com o visor de arrasto + instrução + botões "Redefinir"/OK/Cancelar.
class AjustarFotoDialog : public QDialog
{
    Q_OBJECT

public:
    // caminhoImagem: arquivo de imagem original (resolução cheia) a ajustar.
    explicit AjustarFotoDialog(const QString &caminhoImagem, QPointF focoInicial, QWidget *parent = nullptr);

    QPointF foco() const;

private:
    VisorArrastoFoto *m_visor;
};
