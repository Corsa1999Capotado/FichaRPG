#pragma once

#include <QDialog>
#include <QVector>

#include "tema.h"

class QListWidget;
class QLineEdit;
class QPushButton;

// Diálogo de temas: lista os presets e temas customizados à esquerda, e um
// editor visual (um botão-amostra de cor por elemento) à direita.
class TemaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TemaDialog(QWidget *parent = nullptr);

private:
    struct CampoCor
    {
        QString rotulo;
        QString Tema::*campo;
    };

    void atualizarListaTemas();
    void carregarTemaNaEdicao(const Tema &tema);
    Tema coletarTemaDaEdicao() const;

    void aoSelecionarTema();
    void aplicarSelecionado();
    void salvarComoNovo();
    void escolherCor(int indiceCampo);
    void atualizarAmostra(int indiceCampo);

    static const QVector<CampoCor> &camposCor();

    QListWidget *m_listaTemas;
    QLineEdit *m_nomeEdit;
    QVector<QPushButton *> m_botoesCor;

    QVector<Tema> m_temasListados;
    Tema m_temaEmEdicao;
};
