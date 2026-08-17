#pragma once

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;

// Tela de configuração/controle da integração com Google Drive (Fase 1):
// Client ID/Secret, conectar/desconectar conta, enviar tudo / baixar tudo
// manualmente. Sem sincronização automática em background ainda.
class GoogleDriveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GoogleDriveDialog(QWidget *parent = nullptr);

private:
    void atualizarStatus();
    void salvarCredenciais();
    void conectar();
    void desconectar();
    void enviarTudo();
    void baixarTudo();

    QLineEdit *m_clientIdEdit;
    QLineEdit *m_clientSecretEdit;
    QLabel *m_statusLabel;
    QPushButton *m_botaoConectar;
    QPushButton *m_botaoDesconectar;
    QPushButton *m_botaoEnviar;
    QPushButton *m_botaoBaixar;
};
