#pragma once

#include <QWidget>

class QTextEdit;
class QPushButton;

// Editor de notas em rich text, estilo Word: negrito, itálico, alinhamento
// por parágrafo, e imagens inseridas (por botão ou colando com Ctrl+V) direto
// no meio do texto — redimensionáveis com a roda do mouse ou os botões 🔍+/🔍−.
// Compartilhado entre a tela de edição e a aba "Notas" da visualização.
class EditorNotas : public QWidget
{
    Q_OBJECT

public:
    explicit EditorNotas(QWidget *parent = nullptr);

    QString paraHtml() const;
    void definirHtml(const QString &conteudo); // aceita HTML rico ou texto puro (fichas antigas)

signals:
    void conteudoAlterado();

private:
    void alternarNegrito(bool ligado);
    void alternarItalico(bool ligado);
    void alinhar(Qt::Alignment alinhamento);
    void inserirImagem();
    void redimensionarImagemAtual(double fator);
    void atualizarBotoesFormatacao();

    QTextEdit *m_texto;
    QPushButton *m_botaoNegrito;
    QPushButton *m_botaoItalico;
    QPushButton *m_botaoEsquerda;
    QPushButton *m_botaoCentro;
    QPushButton *m_botaoDireita;
};
