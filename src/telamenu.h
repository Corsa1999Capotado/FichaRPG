#pragma once

#include <QString>
#include <QVector>
#include <QWidget>

class QGridLayout;
class QTabBar;
class QLineEdit;
class QPushButton;
class QResizeEvent;
class CardFicha;

// Tela inicial: cabeçalho com abas horizontais de pastas + busca, grid de
// cards em foco (sem barra lateral), botão flutuante "+" pra nova ficha.
class TelaMenu : public QWidget
{
    Q_OBJECT

public:
    explicit TelaMenu(QWidget *parent = nullptr);

    void atualizarLista();
    QString categoriaSelecionada() const;

signals:
    void abrirFicha(const QString &caminhoArquivo);
    void editarFicha(const QString &caminhoArquivo);
    void novaFicha();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    enum class FiltroVida { Todos, Critico, Normal, Cheio };
    enum class Ordenacao { Nome, VidaMenorPrimeiro, ModificadoRecente };

    void atualizarCategorias();
    void atualizarCards();
    void abrirMenuPastas();
    void gerenciarTemplates();
    void alternarClaroEscuro();
    void abrirDialogoTemas();
    void abrirFiltroAvancado();
    void abrirPreferencias();
    void abrirGoogleDrive();
    void abrirConta();
    void atualizarBotaoConta();
    void reposicionarBotaoNovo();
    void aplicarFundoGradiente();
    void mostrarToast(const QString &mensagem, const QString &corHex);
    void duplicarFicha(const QString &caminhoArquivo);
    void moverFicha(const QString &caminhoArquivo);
    void exportarFicha(const QString &caminhoArquivo);
    void excluirFichaCard(const QString &caminhoArquivo);

    QTabBar *m_abasCategorias;
    QLineEdit *m_buscaEdit;
    QString m_filtroTexto;
    FiltroVida m_filtroVida = FiltroVida::Todos;
    Ordenacao m_ordenacao = Ordenacao::Nome;
    QWidget *m_containerCards;
    QGridLayout *m_grid;
    QVector<CardFicha *> m_cards;
    QPushButton *m_botaoNovo; // FAB flutuante, filho direto da tela
    QPushButton *m_botaoConta;
};
