#include "temadialog.h"

#include "gerenciadortema.h"

#include <QColorDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

const QVector<TemaDialog::CampoCor> &TemaDialog::camposCor()
{
    static const QVector<CampoCor> campos = {
        {"Fundo", &Tema::corFundo},
        {"Fundo (painéis/abas)", &Tema::corFundoAlt},
        {"Cards", &Tema::corCard},
        {"Cards (hover)", &Tema::corCardHover},
        {"Borda", &Tema::corBorda},
        {"Texto", &Tema::corTexto},
        {"Texto secundário", &Tema::corTextoSecundario},
        {"Destaque (accent)", &Tema::corAccent},
        {"Destaque (hover)", &Tema::corAccentHover},
        {"Sucesso", &Tema::corSucesso},
        {"Alerta", &Tema::corAlerta},
        {"Perigo", &Tema::corPerigo},
    };
    return campos;
}

TemaDialog::TemaDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Temas");
    resize(640, 480);

    QHBoxLayout *layoutRaiz = new QHBoxLayout(this);

    // Coluna esquerda: lista de temas disponíveis
    QVBoxLayout *colunaLista = new QVBoxLayout;
    colunaLista->addWidget(new QLabel("Temas disponíveis:"));
    m_listaTemas = new QListWidget;
    connect(m_listaTemas, &QListWidget::currentRowChanged, this, [this](int) { aoSelecionarTema(); });
    colunaLista->addWidget(m_listaTemas, 1);

    QPushButton *botaoAplicar = new QPushButton("Aplicar");
    botaoAplicar->setProperty("accent", true);
    connect(botaoAplicar, &QPushButton::clicked, this, &TemaDialog::aplicarSelecionado);
    colunaLista->addWidget(botaoAplicar);

    layoutRaiz->addLayout(colunaLista, 2);

    // Coluna direita: editor visual de cores
    QVBoxLayout *colunaEditor = new QVBoxLayout;

    QHBoxLayout *linhaNome = new QHBoxLayout;
    linhaNome->addWidget(new QLabel("Nome do tema:"));
    m_nomeEdit = new QLineEdit;
    linhaNome->addWidget(m_nomeEdit, 1);
    colunaEditor->addLayout(linhaNome);

    QGridLayout *grade = new QGridLayout;
    const QVector<CampoCor> &campos = camposCor();
    for (int i = 0; i < campos.size(); ++i) {
        QPushButton *amostra = new QPushButton;
        amostra->setFixedSize(48, 24);
        connect(amostra, &QPushButton::clicked, this, [this, i]() { escolherCor(i); });

        grade->addWidget(new QLabel(campos[i].rotulo + ":"), i, 0);
        grade->addWidget(amostra, i, 1);
        m_botoesCor.append(amostra);
    }
    colunaEditor->addLayout(grade);
    colunaEditor->addStretch();

    QPushButton *botaoSalvarNovo = new QPushButton("Salvar como novo tema");
    connect(botaoSalvarNovo, &QPushButton::clicked, this, &TemaDialog::salvarComoNovo);
    colunaEditor->addWidget(botaoSalvarNovo);

    QPushButton *botaoFechar = new QPushButton("Fechar");
    connect(botaoFechar, &QPushButton::clicked, this, &TemaDialog::accept);
    colunaEditor->addWidget(botaoFechar);

    layoutRaiz->addLayout(colunaEditor, 3);

    atualizarListaTemas();
}

void TemaDialog::atualizarListaTemas()
{
    m_temasListados = GerenciadorTema::instancia().temasDisponiveis();

    m_listaTemas->blockSignals(true);
    m_listaTemas->clear();
    for (const Tema &t : m_temasListados)
        m_listaTemas->addItem(t.nome);
    m_listaTemas->blockSignals(false);

    const QString nomeAtual = GerenciadorTema::instancia().temaAtual().nome;
    int indice = 0;
    for (int i = 0; i < m_temasListados.size(); ++i) {
        if (m_temasListados[i].nome == nomeAtual) {
            indice = i;
            break;
        }
    }
    m_listaTemas->setCurrentRow(indice);
}

void TemaDialog::aoSelecionarTema()
{
    const int linha = m_listaTemas->currentRow();
    if (linha < 0 || linha >= m_temasListados.size())
        return;
    carregarTemaNaEdicao(m_temasListados[linha]);
}

void TemaDialog::carregarTemaNaEdicao(const Tema &tema)
{
    m_temaEmEdicao = tema;
    m_nomeEdit->setText(tema.nome);
    for (int i = 0; i < camposCor().size(); ++i)
        atualizarAmostra(i);
}

Tema TemaDialog::coletarTemaDaEdicao() const
{
    Tema t = m_temaEmEdicao;
    t.nome = m_nomeEdit->text().trimmed();
    return t;
}

void TemaDialog::escolherCor(int indiceCampo)
{
    const QString atual = m_temaEmEdicao.*(camposCor()[indiceCampo].campo);
    const QColor cor = QColorDialog::getColor(QColor(atual), this, "Escolher cor");
    if (!cor.isValid())
        return;

    m_temaEmEdicao.*(camposCor()[indiceCampo].campo) = cor.name();
    atualizarAmostra(indiceCampo);
}

void TemaDialog::atualizarAmostra(int indiceCampo)
{
    const QString cor = m_temaEmEdicao.*(camposCor()[indiceCampo].campo);
    m_botoesCor[indiceCampo]->setStyleSheet(QString("background-color: %1; border: 1px solid #888888;").arg(cor));
}

void TemaDialog::aplicarSelecionado()
{
    GerenciadorTema::instancia().aplicarTema(coletarTemaDaEdicao());
}

void TemaDialog::salvarComoNovo()
{
    const Tema t = coletarTemaDaEdicao();
    if (t.nome.isEmpty()) {
        QMessageBox::warning(this, "Nome obrigatório", "Dê um nome pro tema antes de salvar.");
        return;
    }

    GerenciadorTema::instancia().salvarComoTemaCustomizado(t);
    atualizarListaTemas();
}
