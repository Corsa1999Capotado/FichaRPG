#pragma once

#include <QMainWindow>

class QStackedWidget;
class TelaMenu;
class TelaEdicao;
class TelaVisualizacao;

// Janela principal: só troca entre a tela de menu, a de visualização e a de edição.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void mostrarMenu();
    void mostrarVisualizacao(const QString &caminhoArquivo);
    void iniciarNovaFicha();
    void abrirEdicaoFicha(const QString &caminhoArquivo);
    void aoSalvarFicha(const QString &caminhoArquivo);
    void aoCancelarEdicao(const QString &caminhoArquivoOriginal);

    QStackedWidget *m_pilha;
    TelaMenu *m_telaMenu;
    TelaEdicao *m_telaEdicao;
    TelaVisualizacao *m_telaVisualizacao;
};
