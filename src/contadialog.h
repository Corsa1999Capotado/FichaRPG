#pragma once

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;

// Tela de conta: login/logout com o Google (Client ID/Secret, conectar,
// status, desconectar). É o lugar único onde a conta é gerenciada — o
// GoogleDriveDialog cuida só das ações de sincronização (enviar/baixar tudo),
// e fica desabilitado enquanto não houver conta conectada aqui.
class ContaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ContaDialog(QWidget *parent = nullptr);

private:
    void atualizarStatus();
    void salvarCredenciais();
    void conectar();
    void sairDaConta();

    QLabel *m_avatarLabel;
    QLabel *m_statusLabel;
    QLineEdit *m_clientIdEdit;
    QLineEdit *m_clientSecretEdit;
    QPushButton *m_botaoConectar;
    QPushButton *m_botaoSair;
};
