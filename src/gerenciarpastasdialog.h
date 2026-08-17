#pragma once

#include <QDialog>

class QVBoxLayout;

// Modal de gerenciamento de pastas: reordenar (▲▼), renomear, mudar ícone,
// adicionar e excluir (com opção de mover as fichas antes, se a pasta não
// estiver vazia).
class GerenciarPastasDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GerenciarPastasDialog(QWidget *parent = nullptr);

signals:
    void pastasAlteradas();

private:
    void atualizarLista();
    void adicionarPasta();
    void moverParaCima(const QString &categoria);
    void moverParaBaixo(const QString &categoria);
    void renomear(const QString &categoria);
    void mudarIcone(const QString &categoria);
    void excluir(const QString &categoria);

    QVBoxLayout *m_listaLayout;
};
