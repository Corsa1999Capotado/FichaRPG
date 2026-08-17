#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QEnterEvent;

// Card compacto (195x120) do Painel Lateral de Fichas Rápidas: foto + nome +
// vida, com botões de fechar/reordenar/fixar que só aparecem no hover, e
// destaque de borda quando é a ficha atualmente em foco na visualização.
class CardPainel : public QWidget
{
    Q_OBJECT

public:
    explicit CardPainel(const QString &caminhoArquivo, const QString &nome, const QString &caminhoImagem,
                         int vidaAtual, int vidaMax, bool pinada, const QString &corDestaque, QWidget *parent = nullptr);

    QString caminhoArquivo() const { return m_caminhoArquivo; }

    void definirAtiva(bool ativa);
    void atualizarVida(int atual, int max);
    void definirPodeSubir(bool pode);
    void definirPodeDescer(bool pode);

signals:
    void ativado(const QString &caminhoArquivo);
    void fechado(const QString &caminhoArquivo);
    void moverCima(const QString &caminhoArquivo);
    void moverBaixo(const QString &caminhoArquivo);
    void pinAlternado(const QString &caminhoArquivo);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void aplicarEstiloBase();

    QString m_caminhoArquivo;
    QString m_corDestaque;
    QLabel *m_vidaLabel;
    QPushButton *m_botaoFechar;
    QPushButton *m_botaoCima;
    QPushButton *m_botaoBaixo;
    QPushButton *m_botaoPin;
};
