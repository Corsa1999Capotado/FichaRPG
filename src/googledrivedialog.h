#pragma once

#include <QDialog>

class QLabel;
class QPushButton;

// Tela de sincronização com o Google Drive: enviar tudo / baixar tudo
// manualmente. O login/logout da conta fica no ContaDialog — aqui só fica
// habilitado quando já existe uma conta conectada.
class GoogleDriveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GoogleDriveDialog(QWidget *parent = nullptr);

private:
    void atualizarStatus();
    void enviarTudo();
    void baixarTudo();

    QLabel *m_statusLabel;
    QPushButton *m_botaoEnviar;
    QPushButton *m_botaoBaixar;
};
