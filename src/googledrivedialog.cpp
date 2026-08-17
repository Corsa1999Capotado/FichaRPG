#include "googledrivedialog.h"

#include "googleauth.h"
#include "sincronizadordrive.h"

#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

GoogleDriveDialog::GoogleDriveDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("☁️ Google Drive");
    resize(420, 240);

    QVBoxLayout *layoutRaiz = new QVBoxLayout(this);

    QLabel *explicacao = new QLabel(
        "Faz backup das suas fichas, templates e imagens numa pasta \"FichaRPG\" no seu Google Drive. "
        "Com a conta conectada, toda mudança numa ficha já é enviada automaticamente — os botões abaixo são "
        "só pra um envio/download completo manual (ex: primeira sincronização, ou trazer tudo de outro PC).");
    explicacao->setWordWrap(true);
    layoutRaiz->addWidget(explicacao);

    m_statusLabel = new QLabel;
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet("margin-top: 10px; margin-bottom: 6px; font-weight: bold;");
    layoutRaiz->addWidget(m_statusLabel);

    m_botaoEnviar = new QPushButton("📤 Enviar tudo pro Drive");
    connect(m_botaoEnviar, &QPushButton::clicked, this, &GoogleDriveDialog::enviarTudo);
    layoutRaiz->addWidget(m_botaoEnviar);

    m_botaoBaixar = new QPushButton("📥 Baixar tudo do Drive");
    connect(m_botaoBaixar, &QPushButton::clicked, this, &GoogleDriveDialog::baixarTudo);
    layoutRaiz->addWidget(m_botaoBaixar);

    layoutRaiz->addStretch();

    QPushButton *botaoFechar = new QPushButton("Fechar");
    connect(botaoFechar, &QPushButton::clicked, this, &QDialog::accept);
    layoutRaiz->addWidget(botaoFechar);

    atualizarStatus();
}

void GoogleDriveDialog::atualizarStatus()
{
    const bool conectado = GoogleAuth::estaConectado();
    m_statusLabel->setText(conectado ? QString("☁️ Conectado como %1").arg(GoogleAuth::emailConectado())
                                      : "📴 Nenhuma conta conectada. Conecte no botão \"Conta\" do menu principal.");
    m_botaoEnviar->setEnabled(conectado);
    m_botaoBaixar->setEnabled(conectado);
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
