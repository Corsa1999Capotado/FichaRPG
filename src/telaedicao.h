#pragma once

#include <QVector>
#include <QWidget>

#include "character.h"

class QLineEdit;
class QLabel;
class QVBoxLayout;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class EditorNotas;

// Tela de edição de uma ficha: nome, imagem, idade/altura, vida, discernimento,
// atributos/sub-atributos, inventário e habilidades dinâmicos, e notas livres.
class TelaEdicao : public QWidget
{
    Q_OBJECT

public:
    explicit TelaEdicao(QWidget *parent = nullptr);

    // caminhoArquivoExistente vazio = ficha nova, ainda não salva em disco (nesse
    // caso categoriaNova diz em qual pasta ela deve ser criada ao salvar)
    void carregarFicha(const CharacterSheet &ficha, const QString &caminhoArquivoExistente, const QString &categoriaNova = QString());

signals:
    void salvo(const QString &caminhoArquivo);
    void cancelado(const QString &caminhoArquivoOriginal);

private:
    struct SubAtributoWidgets
    {
        QWidget *linha;
        QLineEdit *nomeEdit;
        QSpinBox *valorSpin;
    };

    struct AtributoWidgets
    {
        QWidget *grupo;
        QLineEdit *nomeEdit;
        QSpinBox *valorSpin;
        QCheckBox *automaticoCheck;
        QLineEdit *formulaEdit;
        QVBoxLayout *subLayout;
        QVector<SubAtributoWidgets> subs;
    };

    struct LinhaNomeDescricaoWidgets
    {
        QWidget *linha;
        QLineEdit *nomeEdit;
        QLineEdit *descricaoEdit;
    };

    struct ItemInventarioWidgets
    {
        QWidget *linha;
        QSpinBox *quantidadeSpin;
        QLineEdit *nomeEdit;
        QLineEdit *utilidadeEdit;
    };

    void montarInterface();

    void limparAtributosUI();
    void adicionarAtributoUI(const Atributo &modelo = Atributo());
    void adicionarSubAtributoUI(AtributoWidgets &atributoWidgets, const SubAtributo &modelo = SubAtributo());
    void removerAtributo(QWidget *grupo);
    void removerSubAtributo(QWidget *grupo, QWidget *linha);
    void moverSubAtributoParaOutroGrupo(QWidget *grupoOrigem, QWidget *linha);
    AtributoWidgets *encontrarAtributoPorGrupo(QWidget *grupo);

    void limparListaUI(QVBoxLayout *layout, QVector<LinhaNomeDescricaoWidgets> &lista);
    void adicionarLinhaNomeDescricaoUI(QVBoxLayout *layout, QVector<LinhaNomeDescricaoWidgets> &lista, const QString &nome, const QString &descricao);
    void removerLinhaNomeDescricao(QVBoxLayout *layout, QVector<LinhaNomeDescricaoWidgets> &lista, QWidget *linha);
    void moverLinhaNomeDescricao(QVBoxLayout *layout, QVector<LinhaNomeDescricaoWidgets> &lista, QWidget *linha, int direcao);

    void limparInventarioUI();
    void adicionarItemInventarioUI(const ItemInventario &modelo = ItemInventario());
    void removerItemInventario(QWidget *linha);
    void moverItemInventario(QWidget *linha, int direcao);

    void preencherCampos(const CharacterSheet &ficha);

    void escolherImagem();
    void atualizarPreviewImagem(const QString &caminho);
    CharacterSheet coletarDaInterface() const;
    void salvar();
    void salvarComoTemplate();
    void restaurarBackup();
    void recalcularFormulas();
    void marcarAlterado();
    void marcarSalvo();

    QLabel *m_avisoAlteracoes;
    QLineEdit *m_nomeEdit;
    QLabel *m_imagemPreview;
    QSpinBox *m_idadeSpin;
    QLineEdit *m_alturaEdit;
    QSpinBox *m_vidaAtualSpin;
    QSpinBox *m_vidaMaxSpin;
    QCheckBox *m_vidaAutomaticaCheck;
    QLineEdit *m_formulaVidaEdit;
    QSpinBox *m_sanidadeAtualSpin;
    QSpinBox *m_sanidadeMaxSpin;
    QCheckBox *m_sanidadeAutomaticaCheck;
    QLineEdit *m_formulaSanidadeEdit;
    QSpinBox *m_discernimentoSpin;
    QCheckBox *m_discernimentoAutomaticoCheck;
    QLineEdit *m_formulaDiscernimentoEdit;
    EditorNotas *m_descricaoEdit;

    QVBoxLayout *m_atributosLayout;
    QVector<AtributoWidgets> m_atributos;

    QDoubleSpinBox *m_dinheiroSpin;
    QVBoxLayout *m_inventarioLayout;
    QVector<ItemInventarioWidgets> m_itensInventario;

    QVBoxLayout *m_habilidadesLayout;
    QVector<LinhaNomeDescricaoWidgets> m_habilidades;

    QString m_caminhoImagemOrigemNova; // imagem escolhida agora, ainda não copiada pra pasta do app
    QString m_imagemArquivoAtual;      // nome do arquivo já salvo em imagens/ (se houver)
    QString m_caminhoArquivoFicha;     // vazio enquanto a ficha nunca foi salva
    QString m_caminhoArquivoOriginal;  // caminho com que a edição começou (vazio se é ficha nova)
    QString m_categoriaNova;           // pasta destino quando a ficha ainda não foi salva
};
