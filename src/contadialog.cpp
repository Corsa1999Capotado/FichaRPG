#include "contadialog.h"

#include "googleauth.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

ContaDialog::ContaDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Conta");
    resize(400, 320);

    QVBoxLayout *layoutRaiz = new QVBoxLayout(this);

    m_avatarLabel = new QLabel("☁️");
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    m_avatarLabel->setStyleSheet("font-size: 40px;");
    layoutRaiz->addWidget(m_avatarLabel);

    m_statusLabel = new QLabel;
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet("font-weight: bold; margin-top: 6px; margin-bottom: 10px;");
    layoutRaiz->addWidget(m_statusLabel);

    QLabel *explicacao = new QLabel(
        "Conecte uma conta do Google pra sincronizar suas fichas, templates e imagens com o Google Drive "
        "e usar em outro PC.");
    explicacao->setWordWrap(true);
    layoutRaiz->addWidget(explicacao);

    QFormLayout *formCredenciais = new QFormLayout;
    m_clientIdEdit = new QLineEdit;
    m_clientIdEdit->setPlaceholderText("Client ID do Google Cloud Console");
    formCredenciais->addRow("Client ID:", m_clientIdEdit);

    m_clientSecretEdit = new QLineEdit;
    m_clientSecretEdit->setPlaceholderText("Client Secret");
    m_clientSecretEdit->setEchoMode(QLineEdit::Password);
    formCredenciais->addRow("Client Secret:", m_clientSecretEdit);
    layoutRaiz->addLayout(formCredenciais);

    QPushButton *botaoSalvarCredenciais = new QPushButton("Salvar credenciais");
    connect(botaoSalvarCredenciais, &QPushButton::clicked, this, &ContaDialog::salvarCredenciais);
    layoutRaiz->addWidget(botaoSalvarCredenciais);

    m_botaoConectar = new QPushButton("🔐 Conectar conta do Google");
    m_botaoConectar->setProperty("accent", true);
    connect(m_botaoConectar, &QPushButton::clicked, this, &ContaDialog::conectar);
    layoutRaiz->addWidget(m_botaoConectar);

    layoutRaiz->addStretch();

    m_botaoSair = new QPushButton("🔓 Sair da conta");
    m_botaoSair->setProperty("danger", true);
    connect(m_botaoSair, &QPushButton::clicked, this, &ContaDialog::sairDaConta);
    layoutRaiz->addWidget(m_botaoSair);

    QPushButton *botaoFechar = new QPushButton("Fechar");
    connect(botaoFechar, &QPushButton::clicked, this, &QDialog::accept);
    layoutRaiz->addWidget(botaoFechar);

    const GoogleAuth::Credenciais cred = GoogleAuth::lerCredenciaisApp();
    m_clientIdEdit->setText(cred.clientId);
    m_clientSecretEdit->setText(cred.clientSecret);

    atualizarStatus();
}

void ContaDialog::atualizarStatus()
{
    const bool conectado = GoogleAuth::estaConectado();
    if (conectado) {
        const QString email = GoogleAuth::emailConectado();
        m_statusLabel->setText(email.isEmpty() ? "☁️ Conectado" : QString("☁️ Conectado como %1").arg(email));
    } else {
        m_statusLabel->setText("📴 Nenhuma conta conectada");
    }

    m_clientIdEdit->setEnabled(!conectado);
    m_clientSecretEdit->setEnabled(!conectado);
    m_botaoConectar->setEnabled(!conectado);
    m_botaoSair->setEnabled(conectado);
}

void ContaDialog::salvarCredenciais()
{
    if (m_clientIdEdit->text().trimmed().isEmpty() || m_clientSecretEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erro", "Preencha o Client ID e o Client Secret.");
        return;
    }
    GoogleAuth::salvarCredenciaisApp(m_clientIdEdit->text(), m_clientSecretEdit->text());
    QMessageBox::information(this, "Salvo", "Credenciais salvas. Agora clique em \"Conectar conta do Google\".");
}

void ContaDialog::conectar()
{
    QMessageBox::information(
        this, "Conectar", "Vou abrir o navegador pra você fazer login no Google. Depois de autorizar, volte pra esse app.");

    const QString erro = GoogleAuth::conectar();
    if (!erro.isEmpty()) {
        QMessageBox::warning(this, "Erro ao conectar", erro);
        return;
    }
    QMessageBox::information(this, "Conectado", "Conta do Google conectada com sucesso!");
    atualizarStatus();
}

void ContaDialog::sairDaConta()
{
    if (QMessageBox::question(this, "Sair da conta",
                               "Desconectar a conta do Google? Você vai precisar logar de novo pra sincronizar com o Drive. "
                               "Seus dados locais continuam intactos.")
        != QMessageBox::Yes)
        return;

    GoogleAuth::desconectar();
    atualizarStatus();
}
