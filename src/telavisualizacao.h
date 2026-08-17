#pragma once

#include <QWidget>

#include "character.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class QGridLayout;
class EditorNotas;
class PainelLateralFichas;

// Tela de visualização (somente leitura, exceto vida/discernimento) de uma
// ficha: imagem/nome/idade/altura/vida/discernimento/recursos à esquerda,
// abas com atributos/sub-atributos/habilidades/inventário/notas à direita,
// tema escuro e botão flutuante de editar.
class TelaVisualizacao : public QWidget
{
    Q_OBJECT

public:
    explicit TelaVisualizacao(QWidget *parent = nullptr);

    void carregarFicha(const CharacterSheet &ficha, const QString &caminhoArquivo);

signals:
    void voltarAoMenu();
    void editarFicha(const QString &caminhoArquivo);
    void fichaExcluida();
    void abrirOutraFicha(const QString &caminhoArquivo);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void montarInterface();
    void preencherConteudo(const CharacterSheet &ficha);
    void reposicionarBotaoEditar();

    void ajustarVida(int delta);
    void ajustarSanidade(int delta);
    void ajustarDiscernimento(int delta);
    void ajustarRecurso(int indice, int delta);
    void adicionarRecurso();
    void removerRecurso(int indice);
    void persistirEAtualizar();
    void exportarFicha();
    void excluirFicha();
    void notasAlteradas();
    void adicionarItemInventario();
    void ajustarQuantidadeItem(int indice, int delta);
    void ajustarDinheiro(bool adicionar);
    void alternarPainelFichas();
    void abrirFichaDoPainel(const QString &caminho);
    void abrirNesimaFichaDoPainel(int indiceUm);
    void adicionarFichaAoPainel();

    QWidget *criarCardAtributo(const QString &nome, int valor, const QString &descricao);
    QWidget *criarLinhaSubAtributo(const QString &nome, int valor);
    QWidget *criarCardItem(const QString &nome, const QString &descricao);
    QWidget *criarLinhaInventario(int indice, int quantidade, const QString &nome, const QString &utilidade, bool contavel);
    QWidget *criarLinhaRecurso(int indice, const QString &nome, int atual, int max);
    bool correspondeAoFiltro(const QString &texto) const;

    QLineEdit *m_buscaEdit;
    QString m_filtroTexto;

    QLabel *m_imagemLabel;
    QLabel *m_nomeLabel;
    QLabel *m_idadeLabel;
    QLabel *m_alturaLabel;
    QLabel *m_vidaLabel;
    QLabel *m_sanidadeLabel;
    QLabel *m_discernimentoLabel;
    QLabel *m_dinheiroLabel;

    QVBoxLayout *m_recursosLayout;
    QGridLayout *m_atributosGrid;
    QVBoxLayout *m_subAtributosLayout;
    QVBoxLayout *m_habilidadesLayout;
    QVBoxLayout *m_inventarioLayout;
    EditorNotas *m_notasEdit;

    QPushButton *m_botaoEditar;
    QPushButton *m_botaoPainel;
    PainelLateralFichas *m_painelFichas;
    bool m_mostrarAoAbrirAplicado = false;

    CharacterSheet m_fichaAtual;
    QString m_caminhoArquivoAtual;
};
