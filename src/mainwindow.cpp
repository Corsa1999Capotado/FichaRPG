#include "mainwindow.h"

#include "armazenamento.h"
#include "character.h"
#include "fichatemplate.h"
#include "telaedicao.h"
#include "telamenu.h"
#include "telavisualizacao.h"

#include <QAbstractButton>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMap>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>

namespace
{
// Mostra um diálogo com os templates disponíveis e devolve o caminho do
// arquivo escolhido (vazio se cancelado ou se não houver nenhum template).
QString escolherTemplate(QWidget *pai)
{
    const QStringList arquivos = Armazenamento::listarArquivosTemplates();

    QStringList nomesExibidos;
    QMap<QString, QString> nomeParaCaminho;
    for (const QString &caminho : arquivos) {
        FichaTemplate t;
        if (!FichaTemplate::carregarDeArquivo(caminho, t))
            continue;

        QString nomeExibido = t.nomeTemplate.isEmpty() ? QFileInfo(caminho).baseName() : t.nomeTemplate;
        while (nomeParaCaminho.contains(nomeExibido))
            nomeExibido += " ";
        nomesExibidos << nomeExibido;
        nomeParaCaminho[nomeExibido] = caminho;
    }

    if (nomesExibidos.isEmpty()) {
        QMessageBox::information(pai, "Nenhum template", "Ainda não há nenhum template salvo.");
        return QString();
    }

    bool ok = false;
    const QString escolhido = QInputDialog::getItem(pai, "Escolher template", "Template:", nomesExibidos, 0, false, &ok);
    if (!ok)
        return QString();

    return nomeParaCaminho.value(escolhido);
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Ficha de RPG");
    resize(1100, 760);

    m_pilha = new QStackedWidget(this);
    m_telaMenu = new TelaMenu(this);
    m_telaVisualizacao = new TelaVisualizacao(this);
    m_telaEdicao = new TelaEdicao(this);

    m_pilha->addWidget(m_telaMenu);
    m_pilha->addWidget(m_telaVisualizacao);
    m_pilha->addWidget(m_telaEdicao);
    setCentralWidget(m_pilha);

    connect(m_telaMenu, &TelaMenu::novaFicha, this, &MainWindow::iniciarNovaFicha);
    connect(m_telaMenu, &TelaMenu::abrirFicha, this, &MainWindow::mostrarVisualizacao);
    connect(m_telaMenu, &TelaMenu::editarFicha, this, &MainWindow::abrirEdicaoFicha);

    connect(m_telaVisualizacao, &TelaVisualizacao::voltarAoMenu, this, &MainWindow::mostrarMenu);
    connect(m_telaVisualizacao, &TelaVisualizacao::editarFicha, this, &MainWindow::abrirEdicaoFicha);
    connect(m_telaVisualizacao, &TelaVisualizacao::fichaExcluida, this, &MainWindow::mostrarMenu);
    connect(m_telaVisualizacao, &TelaVisualizacao::abrirOutraFicha, this, &MainWindow::mostrarVisualizacao);

    connect(m_telaEdicao, &TelaEdicao::salvo, this, &MainWindow::aoSalvarFicha);
    connect(m_telaEdicao, &TelaEdicao::cancelado, this, &MainWindow::aoCancelarEdicao);

    // Garante que sempre exista pelo menos o template ARCA pra escolher ao criar ficha nova
    if (Armazenamento::listarArquivosTemplates().isEmpty()) {
        FichaTemplate arca;
        arca.nomeTemplate = "ARCA";
        arca.base = CharacterSheet::modeloArca();
        arca.salvarEmArquivo(Armazenamento::gerarNomeArquivoTemplateUnico("ARCA"));
    }

    mostrarMenu();
}

void MainWindow::mostrarMenu()
{
    m_telaMenu->atualizarLista();
    m_pilha->setCurrentWidget(m_telaMenu);
}

void MainWindow::mostrarVisualizacao(const QString &caminhoArquivo)
{
    CharacterSheet ficha;
    if (!CharacterSheet::carregarDeArquivo(caminhoArquivo, ficha)) {
        QMessageBox::warning(this, "Erro", "Não foi possível abrir a ficha selecionada.");
        mostrarMenu();
        return;
    }

    m_telaVisualizacao->carregarFicha(ficha, caminhoArquivo);
    m_pilha->setCurrentWidget(m_telaVisualizacao);
}

void MainWindow::iniciarNovaFicha()
{
    const QString categoria = m_telaMenu->categoriaSelecionada();
    if (categoria.isEmpty()) {
        QMessageBox::warning(this, "Nenhuma pasta", "Crie ou selecione uma pasta antes de criar uma ficha.");
        return;
    }

    QMessageBox caixa(this);
    caixa.setWindowTitle("Nova ficha");
    caixa.setText(QString("Como você quer criar a ficha (pasta \"%1\")?").arg(categoria));
    QPushButton *botaoTxt = caixa.addButton("Importar de .txt", QMessageBox::ActionRole);
    QPushButton *botaoTemplate = caixa.addButton("Usar template", QMessageBox::ActionRole);
    QPushButton *botaoZero = caixa.addButton("Criar do zero", QMessageBox::ActionRole);
    caixa.addButton(QMessageBox::Cancel);
    caixa.exec();

    QAbstractButton *escolhido = caixa.clickedButton();

    CharacterSheet ficha;
    if (escolhido == botaoTxt) {
        const QString caminho = QFileDialog::getOpenFileName(this, "Importar ficha", QString(), "Texto (*.txt)");
        if (caminho.isEmpty())
            return;

        QFile arquivo(caminho);
        if (!arquivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Erro", "Não foi possível abrir o arquivo selecionado.");
            return;
        }
        const QString conteudo = QString::fromUtf8(arquivo.readAll());
        arquivo.close();

        bool usouFormatoGenerico = false;
        ficha = CharacterSheet::importarDeTexto(conteudo, &usouFormatoGenerico);

        if (usouFormatoGenerico) {
            const bool querSalvar = QMessageBox::question(
                                         this, "Formato não reconhecido",
                                         "Esse .txt não segue nenhum modelo que você já tem salvo, então tentei "
                                         "identificar os atributos automaticamente (pares \"Nome: número\"). "
                                         "Quer salvar essa estrutura como um novo template pra reusar depois?")
                                     == QMessageBox::Yes;
            if (querSalvar) {
                bool ok = false;
                const QString nomeTemplate = QInputDialog::getText(this, "Novo template", "Nome do template:", QLineEdit::Normal, "Importado", &ok);
                if (ok && !nomeTemplate.trimmed().isEmpty()) {
                    FichaTemplate t;
                    t.nomeTemplate = nomeTemplate.trimmed();
                    t.base = ficha;
                    t.base.nome.clear();
                    t.salvarEmArquivo(Armazenamento::gerarNomeArquivoTemplateUnico(t.nomeTemplate));
                }
            }
        }
    } else if (escolhido == botaoTemplate) {
        const QString caminhoTemplate = escolherTemplate(this);
        if (caminhoTemplate.isEmpty())
            return;

        FichaTemplate t;
        if (!FichaTemplate::carregarDeArquivo(caminhoTemplate, t)) {
            QMessageBox::warning(this, "Erro", "Não foi possível carregar o template escolhido.");
            return;
        }
        ficha = t.base;
        ficha.nome.clear();
        ficha.imagemArquivo.clear();
    } else if (escolhido == botaoZero) {
        ficha = CharacterSheet();
    } else {
        return; // cancelado
    }

    m_telaEdicao->carregarFicha(ficha, QString(), categoria);
    m_pilha->setCurrentWidget(m_telaEdicao);
}

void MainWindow::abrirEdicaoFicha(const QString &caminhoArquivo)
{
    CharacterSheet ficha;
    if (!CharacterSheet::carregarDeArquivo(caminhoArquivo, ficha)) {
        QMessageBox::warning(this, "Erro", "Não foi possível abrir a ficha selecionada.");
        return;
    }

    m_telaEdicao->carregarFicha(ficha, caminhoArquivo);
    m_pilha->setCurrentWidget(m_telaEdicao);
}

void MainWindow::aoSalvarFicha(const QString &caminhoArquivo)
{
    mostrarVisualizacao(caminhoArquivo);
}

void MainWindow::aoCancelarEdicao(const QString &caminhoArquivoOriginal)
{
    if (caminhoArquivoOriginal.isEmpty())
        mostrarMenu();
    else
        mostrarVisualizacao(caminhoArquivoOriginal);
}
