#include "gerenciarpastasdialog.h"

#include "armazenamento.h"
#include "preferencias.h"

#include <QAbstractButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLayoutItem>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace
{
const QStringList kIconesPreset = {"👥", "🧛", "🤖", "🐉", "🧙", "⚔️", "🏰", "🌲", "💀", "📁"};
}

GerenciarPastasDialog::GerenciarPastasDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Gerenciar pastas");
    resize(440, 480);

    QVBoxLayout *layoutRaiz = new QVBoxLayout(this);

    QWidget *containerLista = new QWidget;
    m_listaLayout = new QVBoxLayout(containerLista);
    m_listaLayout->setAlignment(Qt::AlignTop);
    m_listaLayout->setSpacing(6);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(containerLista);
    layoutRaiz->addWidget(scroll, 1);

    QPushButton *botaoAdicionar = new QPushButton("➕ Adicionar pasta");
    botaoAdicionar->setProperty("accent", true);
    connect(botaoAdicionar, &QPushButton::clicked, this, &GerenciarPastasDialog::adicionarPasta);
    layoutRaiz->addWidget(botaoAdicionar);

    QPushButton *botaoFechar = new QPushButton("Fechar");
    connect(botaoFechar, &QPushButton::clicked, this, &QDialog::accept);
    layoutRaiz->addWidget(botaoFechar);

    atualizarLista();
}

void GerenciarPastasDialog::atualizarLista()
{
    QLayoutItem *item;
    while ((item = m_listaLayout->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    const QStringList categorias = Armazenamento::listarCategorias();
    for (int i = 0; i < categorias.size(); ++i) {
        const QString categoria = categorias[i];
        const QString icone = Armazenamento::iconeCategoria(categoria);
        const int totalFichas = Armazenamento::listarArquivosFichas(categoria).size();

        QWidget *linha = new QWidget;
        linha->setProperty("card", true);
        QHBoxLayout *layoutLinha = new QHBoxLayout(linha);
        layoutLinha->setContentsMargins(8, 6, 8, 6);

        QPushButton *botaoCima = new QPushButton("▲");
        botaoCima->setProperty("compact", true);
        botaoCima->setFixedWidth(28);
        botaoCima->setEnabled(i > 0);
        connect(botaoCima, &QPushButton::clicked, this, [this, categoria]() { moverParaCima(categoria); });
        layoutLinha->addWidget(botaoCima);

        QPushButton *botaoBaixo = new QPushButton("▼");
        botaoBaixo->setProperty("compact", true);
        botaoBaixo->setFixedWidth(28);
        botaoBaixo->setEnabled(i < categorias.size() - 1);
        connect(botaoBaixo, &QPushButton::clicked, this, [this, categoria]() { moverParaBaixo(categoria); });
        layoutLinha->addWidget(botaoBaixo);

        QLabel *label = new QLabel(QString("%1 %2  (%3 ficha%4)")
                                        .arg(icone.isEmpty() ? "📁" : icone, categoria)
                                        .arg(totalFichas)
                                        .arg(totalFichas == 1 ? "" : "s"));
        layoutLinha->addWidget(label, 1);

        QPushButton *botaoRenomear = new QPushButton("✎");
        botaoRenomear->setProperty("compact", true);
        botaoRenomear->setFixedWidth(28);
        botaoRenomear->setToolTip("Renomear");
        connect(botaoRenomear, &QPushButton::clicked, this, [this, categoria]() { renomear(categoria); });
        layoutLinha->addWidget(botaoRenomear);

        QPushButton *botaoIcone = new QPushButton("🎨");
        botaoIcone->setProperty("compact", true);
        botaoIcone->setFixedWidth(28);
        botaoIcone->setToolTip("Mudar ícone");
        connect(botaoIcone, &QPushButton::clicked, this, [this, categoria]() { mudarIcone(categoria); });
        layoutLinha->addWidget(botaoIcone);

        QPushButton *botaoExcluir = new QPushButton("🗑");
        botaoExcluir->setProperty("compact", true);
        botaoExcluir->setProperty("danger", true);
        botaoExcluir->setFixedWidth(28);
        botaoExcluir->setToolTip("Excluir");
        connect(botaoExcluir, &QPushButton::clicked, this, [this, categoria]() { excluir(categoria); });
        layoutLinha->addWidget(botaoExcluir);

        m_listaLayout->addWidget(linha);
    }
}

void GerenciarPastasDialog::adicionarPasta()
{
    bool ok = false;
    const QString nome = QInputDialog::getText(this, "Nova pasta", "Nome da pasta:", QLineEdit::Normal, QString(), &ok);
    if (!ok || nome.trimmed().isEmpty())
        return;

    bool okIcone = false;
    const QString icone = QInputDialog::getItem(this, "Ícone da pasta", "Escolha um ícone:", kIconesPreset, 0, true, &okIcone);

    if (!Armazenamento::criarCategoria(nome.trimmed(), okIcone ? icone.trimmed() : QString())) {
        QMessageBox::warning(this, "Erro", "Não foi possível criar a pasta (nome inválido ou reservado).");
        return;
    }
    atualizarLista();
    emit pastasAlteradas();
}

void GerenciarPastasDialog::moverParaCima(const QString &categoria)
{
    QStringList categorias = Armazenamento::listarCategorias();
    const int i = categorias.indexOf(categoria);
    if (i <= 0)
        return;
    categorias.swapItemsAt(i, i - 1);
    Armazenamento::reordenarCategorias(categorias);
    atualizarLista();
    emit pastasAlteradas();
}

void GerenciarPastasDialog::moverParaBaixo(const QString &categoria)
{
    QStringList categorias = Armazenamento::listarCategorias();
    const int i = categorias.indexOf(categoria);
    if (i < 0 || i >= categorias.size() - 1)
        return;
    categorias.swapItemsAt(i, i + 1);
    Armazenamento::reordenarCategorias(categorias);
    atualizarLista();
    emit pastasAlteradas();
}

void GerenciarPastasDialog::renomear(const QString &categoria)
{
    bool ok = false;
    const QString novoNome = QInputDialog::getText(this, "Renomear pasta", "Novo nome:", QLineEdit::Normal, categoria, &ok);
    if (!ok || novoNome.trimmed().isEmpty() || novoNome.trimmed() == categoria)
        return;

    if (!Armazenamento::renomearCategoria(categoria, novoNome.trimmed())) {
        QMessageBox::warning(this, "Erro", "Não foi possível renomear (nome inválido, reservado ou já em uso).");
        return;
    }
    atualizarLista();
    emit pastasAlteradas();
}

void GerenciarPastasDialog::mudarIcone(const QString &categoria)
{
    bool ok = false;
    const QString icone = QInputDialog::getItem(this, "Ícone da pasta", QString("Ícone de \"%1\":").arg(categoria), kIconesPreset, 0, true, &ok);
    if (!ok)
        return;
    Armazenamento::definirIconeCategoria(categoria, icone.trimmed());
    atualizarLista();
    emit pastasAlteradas();
}

void GerenciarPastasDialog::excluir(const QString &categoria)
{
    const QStringList todasCategorias = Armazenamento::listarCategorias();
    if (todasCategorias.size() <= 1) {
        QMessageBox::warning(this, "Não é possível excluir", "Precisa existir pelo menos uma pasta — crie outra antes de excluir essa.");
        return;
    }

    const int totalFichas = Armazenamento::listarArquivosFichas(categoria).size();

    if (totalFichas == 0) {
        if (Preferencias::confirmarAntesDeExcluir()
            && QMessageBox::question(this, "Excluir pasta", QString("Excluir a pasta \"%1\"?").arg(categoria)) != QMessageBox::Yes)
            return;
        Armazenamento::excluirCategoria(categoria);
        atualizarLista();
        emit pastasAlteradas();
        return;
    }

    QMessageBox caixa(this);
    caixa.setWindowTitle("Excluir pasta");
    caixa.setText(QString("A pasta \"%1\" tem %2 ficha(s) dentro. O que fazer com elas?").arg(categoria).arg(totalFichas));
    QPushButton *botaoMover = caixa.addButton("🔄 Mover pra outra pasta", QMessageBox::ActionRole);
    QPushButton *botaoExcluirTudo = caixa.addButton("🗑️ Excluir tudo junto", QMessageBox::DestructiveRole);
    caixa.addButton(QMessageBox::Cancel);
    caixa.exec();

    QAbstractButton *escolhido = caixa.clickedButton();
    if (escolhido == botaoMover) {
        QStringList destinos = todasCategorias;
        destinos.removeAll(categoria);
        bool ok = false;
        const QString destino = QInputDialog::getItem(this, "Mover fichas para", "Pasta de destino:", destinos, 0, false, &ok);
        if (!ok)
            return;
        Armazenamento::moverTodasFichas(categoria, destino);
        Armazenamento::excluirCategoria(categoria);
    } else if (escolhido == botaoExcluirTudo) {
        if (Preferencias::confirmarAntesDeExcluir()
            && QMessageBox::question(this, "Confirmar exclusão", QString("Tem certeza? Isso apaga %1 ficha(s) permanentemente.").arg(totalFichas))
                   != QMessageBox::Yes)
            return;
        Armazenamento::excluirCategoria(categoria);
    } else {
        return;
    }
    atualizarLista();
    emit pastasAlteradas();
}
