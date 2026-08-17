#include "googledrivedialog.h"

#include "googleauth.h"
#include "sincronizadordrive.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

GoogleDriveDialog::GoogleDriveDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("☁️ Google Drive");
    resize(440, 360);

    QVBoxLayout *layoutRaiz = new QVBoxLayout(this);

    QLabel *explicacao = new QLabel(
        "Faz backup das suas fichas, templates e imagens numa pasta \"FichaRPG\" no seu Google Drive. "
        "Por enquanto a sincronização é manual (\"Enviar tudo\" / \"Baixar tudo\") — sem automação em segundo plano ainda.");
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
    connect(botaoSalvarCredenciais, &QPushButton::clicked, this, &GoogleDriveDialog::salvarCredenciais);
    layoutRaiz->addWidget(botaoSalvarCredenciais);

    m_statusLabel = new QLabel;
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet("margin-top: 10px; font-weight: bold;");
    layoutRaiz->addWidget(m_statusLabel);

    m_botaoConectar = new QPushButton("🔐 Conectar Google Drive");
    m_botaoConectar->setProperty("accent", true);
    connect(m_botaoConectar, &QPushButton::clicked, this, &GoogleDriveDialog::conectar);
    layoutRaiz->addWidget(m_botaoConectar);

    m_botaoDesconectar = new QPushButton("🔓 Desconectar");
    m_botaoDesconectar->setProperty("danger", true);
    connect(m_botaoDesconectar, &QPushButton::clicked, this, &GoogleDriveDialog::desconectar);
    layoutRaiz->addWidget(m_botaoDesconectar);

    layoutRaiz->addSpacing(10);

    m_botaoEnviar = new QPushButton("📤 Enviar tudo pro Drive");
    connect(m_botaoEnviar, &QPushButton::clicked, this, &GoogleDriveDialog::enviarTudo);
    layoutRaiz->addWidget(m_botaoEnviar);

    m_botaoBaixar = new QPushButton("📥 Baixar tudo do Drive");
    connect(m_botaoBaixar, &QPushButton::clicked, this, &GoogleDriveDialog::baixarTudo);
    layoutRaiz->addWidget(m_botaoBaixar);

    const GoogleAuth::Credenciais cred = GoogleAuth::lerCredenciaisApp();
    m_clientIdEdit->setText(cred.clientId);
    m_clientSecretEdit->setText(cred.clientSecret);

    atualizarStatus();
}

void GoogleDriveDialog::atualizarStatus()
{
    const bool conectado = GoogleAuth::estaConectado();
    m_statusLabel->setText(conectado ? QString("☁️ Conectado como %1").arg(GoogleAuth::emailConectado()) : "📴 Não conectado");

    m_botaoConectar->setEnabled(!conectado);
    m_botaoDesconectar->setEnabled(conectado);
    m_botaoEnviar->setEnabled(conectado);
    m_botaoBaixar->setEnabled(conectado);
}

void GoogleDriveDialog::salvarCredenciais()
{
    if (m_clientIdEdit->text().trimmed().isEmpty() || m_clientSecretEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erro", "Preencha o Client ID e o Client Secret.");
        return;
    }
    GoogleAuth::salvarCredenciaisApp(m_clientIdEdit->text(), m_clientSecretEdit->text());
    QMessageBox::information(this, "Salvo", "Credenciais salvas. Agora clique em \"Conectar Google Drive\".");
}

void GoogleDriveDialog::conectar()
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

void GoogleDriveDialog::desconectar()
{
    if (QMessageBox::question(this, "Desconectar", "Desconectar a conta do Google Drive? Seus dados locais continuam intactos.")
        != QMessageBox::Yes)
        return;
    GoogleAuth::desconectar();
    atualizarStatus();
}

void GoogleDriveDialog::enviarTudo()
{
    if (QMessageBox::question(
            this, "Enviar tudo",
            "Enviar todas as fichas, templates e imagens locais pro Drive? Isso pode sobrescrever arquivos que já estão lá com o mesmo nome.")
        != QMessageBox::Yes)
        return;

    const QString erro = SincronizadorDrive::enviarTudo(this);
    if (!erro.isEmpty())
        QMessageBox::warning(this, "Erro no envio", erro);
    else
        QMessageBox::information(this, "Concluído", "Tudo enviado pro Google Drive.");
}

void GoogleDriveDialog::baixarTudo()
{
    if (QMessageBox::question(this, "Baixar tudo",
                               "Baixar tudo do Drive? Isso SOBRESCREVE os arquivos locais com o mesmo nome pela versão da nuvem. "
                               "Fichas editadas localmente e ainda não enviadas serão perdidas.")
        != QMessageBox::Yes)
        return;

    const QString erro = SincronizadorDrive::baixarTudo(this);
    if (!erro.isEmpty())
        QMessageBox::warning(this, "Erro no download", erro);
    else
        QMessageBox::information(this, "Concluído", "Tudo baixado do Google Drive.");
}
