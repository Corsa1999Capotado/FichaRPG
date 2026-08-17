#include "telavisualizacao.h"

#include "armazenamento.h"
#include "editornotas.h"
#include "exportador.h"
#include "gerenciadortema.h"
#include "imagemutil.h"
#include "painelfichas.h"
#include "painellateralfichas.h"
#include "preferencias.h"

#include <QAbstractButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMap>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

namespace
{
void limparLayout(QLayout *layout)
{
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (QWidget *w = item->widget())
            w->deleteLater();
        delete item;
    }
}

QString corParaValor(const Tema &tema, int valor)
{
    if (valor <= 1)
        return tema.corPerigo;
    if (valor >= 7)
        return tema.corAlerta;
    return tema.corTexto;
}

QString corParaVida(const Tema &tema, int atual, int max)
{
    if (max <= 0)
        return tema.corTexto;
    const double razao = double(atual) / double(max);
    return razao <= 0.3 ? tema.corPerigo : tema.corSucesso;
}

QString corParaDiscernimento(const Tema &tema, int valor)
{
    if (valor >= 80)
        return tema.corPerigo;
    if (valor >= 50)
        return tema.corAlerta;
    return tema.corTexto;
}

QPushButton *criarBotaoAjuste(const QString &texto)
{
    QPushButton *botao = new QPushButton(texto);
    botao->setFixedSize(26, 26);
    botao->setCursor(Qt::PointingHandCursor);
    botao->setProperty("compact", true);
    return botao;
}
}

TelaVisualizacao::TelaVisualizacao(QWidget *parent)
    : QWidget(parent)
{
    montarInterface();
    connect(&GerenciadorTema::instancia(), &GerenciadorTema::temaAlterado, this, [this](const Tema &) {
        preencherConteudo(m_fichaAtual);
        m_painelFichas->atualizarLista(m_caminhoArquivoAtual); // recria os cards com a nova cor de destaque (accent)
    });

    QShortcut *atalhoAltP = new QShortcut(QKeySequence("Alt+P"), this);
    connect(atalhoAltP, &QShortcut::activated, this, &TelaVisualizacao::alternarPainelFichas);
    QShortcut *atalhoCtrlShiftL = new QShortcut(QKeySequence("Ctrl+Shift+L"), this);
    connect(atalhoCtrlShiftL, &QShortcut::activated, this, &TelaVisualizacao::alternarPainelFichas);

    QShortcut *atalhoEsc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(atalhoEsc, &QShortcut::activated, this, [this]() { m_painelFichas->fechar(); });

    for (int i = 1; i <= 5; ++i) {
        QShortcut *atalhoNumero = new QShortcut(QKeySequence(QString("Alt+%1").arg(i)), this);
        connect(atalhoNumero, &QShortcut::activated, this, [this, i]() { abrirNesimaFichaDoPainel(i); });
    }
}

void TelaVisualizacao::montarInterface()
{
    QVBoxLayout *layoutRaiz = new QVBoxLayout(this);

    QHBoxLayout *barraSuperior = new QHBoxLayout;

    m_botaoPainel = new QPushButton("📋");
    m_botaoPainel->setToolTip("Fichas abertas (Alt+P)");
    m_botaoPainel->setFixedWidth(36);
    connect(m_botaoPainel, &QPushButton::clicked, this, &TelaVisualizacao::alternarPainelFichas);
    barraSuperior->addWidget(m_botaoPainel);

    QPushButton *botaoVoltar = new QPushButton("← Voltar");
    connect(botaoVoltar, &QPushButton::clicked, this, &TelaVisualizacao::voltarAoMenu);
    barraSuperior->addWidget(botaoVoltar);

    QPushButton *botaoExportar = new QPushButton("📤 Exportar");
    connect(botaoExportar, &QPushButton::clicked, this, &TelaVisualizacao::exportarFicha);
    barraSuperior->addWidget(botaoExportar);

    QPushButton *botaoExcluir = new QPushButton("🗑 Excluir");
    botaoExcluir->setProperty("danger", true);
    connect(botaoExcluir, &QPushButton::clicked, this, &TelaVisualizacao::excluirFicha);
    barraSuperior->addWidget(botaoExcluir);

    barraSuperior->addStretch();

    barraSuperior->addWidget(new QLabel("🔍 Buscar:"));
    m_buscaEdit = new QLineEdit;
    m_buscaEdit->setPlaceholderText("Nome ou valor...");
    m_buscaEdit->setFixedWidth(220);
    connect(m_buscaEdit, &QLineEdit::textChanged, this, [this](const QString &texto) {
        m_filtroTexto = texto;
        preencherConteudo(m_fichaAtual);
    });
    barraSuperior->addWidget(m_buscaEdit);

    layoutRaiz->addLayout(barraSuperior);

    QHBoxLayout *corpo = new QHBoxLayout;

    // Coluna esquerda (~30%): Foto, Nome, Idade, Altura, Vida, Discernimento
    QWidget *containerEsquerda = new QWidget;
    QVBoxLayout *colunaEsquerda = new QVBoxLayout(containerEsquerda);

    m_imagemLabel = new QLabel;
    m_imagemLabel->setFixedSize(240, 240);
    m_imagemLabel->setAlignment(Qt::AlignCenter);
    m_imagemLabel->setProperty("card", true);
    m_imagemLabel->setText("Sem imagem");

    m_nomeLabel = new QLabel("Sem nome");
    m_nomeLabel->setAlignment(Qt::AlignCenter);
    m_nomeLabel->setWordWrap(true);
    m_nomeLabel->setStyleSheet("font-size: 22px; font-weight: bold; margin-top: 10px;");

    m_idadeLabel = new QLabel;
    m_idadeLabel->setAlignment(Qt::AlignCenter);
    m_idadeLabel->setStyleSheet("font-size: 13px; margin-top: 4px;");

    m_alturaLabel = new QLabel;
    m_alturaLabel->setAlignment(Qt::AlignCenter);
    m_alturaLabel->setStyleSheet("font-size: 13px;");

    // Vida com +/-
    QHBoxLayout *linhaVida = new QHBoxLayout;
    QPushButton *botaoVidaMenos = criarBotaoAjuste("-");
    QPushButton *botaoVidaMais = criarBotaoAjuste("+");
    m_vidaLabel = new QLabel("Vida: 0 / 0");
    m_vidaLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    connect(botaoVidaMenos, &QPushButton::clicked, this, [this]() { ajustarVida(-1); });
    connect(botaoVidaMais, &QPushButton::clicked, this, [this]() { ajustarVida(1); });
    linhaVida->addStretch();
    linhaVida->addWidget(botaoVidaMenos);
    linhaVida->addWidget(m_vidaLabel);
    linhaVida->addWidget(botaoVidaMais);
    linhaVida->addStretch();

    // Sanidade com +/-
    QHBoxLayout *linhaSanidade = new QHBoxLayout;
    QPushButton *botaoSanMenos = criarBotaoAjuste("-");
    QPushButton *botaoSanMais = criarBotaoAjuste("+");
    m_sanidadeLabel = new QLabel("Sanidade: 0 / 0");
    m_sanidadeLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    connect(botaoSanMenos, &QPushButton::clicked, this, [this]() { ajustarSanidade(-1); });
    connect(botaoSanMais, &QPushButton::clicked, this, [this]() { ajustarSanidade(1); });
    linhaSanidade->addStretch();
    linhaSanidade->addWidget(botaoSanMenos);
    linhaSanidade->addWidget(m_sanidadeLabel);
    linhaSanidade->addWidget(botaoSanMais);
    linhaSanidade->addStretch();

    // Discernimento com +/-
    QHBoxLayout *linhaDiscernimento = new QHBoxLayout;
    QPushButton *botaoDiscMenos = criarBotaoAjuste("-");
    QPushButton *botaoDiscMais = criarBotaoAjuste("+");
    m_discernimentoLabel = new QLabel("Discernimento: 0%");
    m_discernimentoLabel->setStyleSheet("font-size: 15px; font-weight: bold;");
    connect(botaoDiscMenos, &QPushButton::clicked, this, [this]() { ajustarDiscernimento(-1); });
    connect(botaoDiscMais, &QPushButton::clicked, this, [this]() { ajustarDiscernimento(1); });
    linhaDiscernimento->addStretch();
    linhaDiscernimento->addWidget(botaoDiscMenos);
    linhaDiscernimento->addWidget(m_discernimentoLabel);
    linhaDiscernimento->addWidget(botaoDiscMais);
    linhaDiscernimento->addStretch();

    // Recursos personalizados (definidos pelo mestre, além de vida/sanidade/discernimento)
    m_recursosLayout = new QVBoxLayout;
    QPushButton *botaoAddRecurso = new QPushButton("➕ Adicionar recurso");
    botaoAddRecurso->setProperty("compact", true);
    connect(botaoAddRecurso, &QPushButton::clicked, this, &TelaVisualizacao::adicionarRecurso);
    QHBoxLayout *linhaAddRecurso = new QHBoxLayout;
    linhaAddRecurso->addStretch();
    linhaAddRecurso->addWidget(botaoAddRecurso);
    linhaAddRecurso->addStretch();

    colunaEsquerda->addStretch();
    colunaEsquerda->addWidget(m_imagemLabel, 0, Qt::AlignHCenter);
    colunaEsquerda->addWidget(m_nomeLabel);
    colunaEsquerda->addWidget(m_idadeLabel);
    colunaEsquerda->addWidget(m_alturaLabel);
    colunaEsquerda->addSpacing(16);
    colunaEsquerda->addLayout(linhaVida);
    colunaEsquerda->addLayout(linhaSanidade);
    colunaEsquerda->addLayout(linhaDiscernimento);
    colunaEsquerda->addSpacing(10);
    colunaEsquerda->addLayout(m_recursosLayout);
    colunaEsquerda->addLayout(linhaAddRecurso);
    colunaEsquerda->addStretch();

    // Coluna direita (~70%) — abas
    QTabWidget *abas = new QTabWidget;

    QWidget *paginaAtributos = new QWidget;
    m_atributosGrid = new QGridLayout(paginaAtributos);
    m_atributosGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_atributosGrid->setSpacing(12);
    QScrollArea *scrollAtributos = new QScrollArea;
    scrollAtributos->setWidgetResizable(true);
    scrollAtributos->setWidget(paginaAtributos);
    abas->addTab(scrollAtributos, "📊 Atributos");

    QWidget *paginaSubAtributos = new QWidget;
    m_subAtributosLayout = new QVBoxLayout(paginaSubAtributos);
    m_subAtributosLayout->setAlignment(Qt::AlignTop);
    QScrollArea *scrollSubAtributos = new QScrollArea;
    scrollSubAtributos->setWidgetResizable(true);
    scrollSubAtributos->setWidget(paginaSubAtributos);
    abas->addTab(scrollSubAtributos, "📈 Sub-Atributos");

    QWidget *paginaHabilidades = new QWidget;
    m_habilidadesLayout = new QVBoxLayout(paginaHabilidades);
    m_habilidadesLayout->setAlignment(Qt::AlignTop);
    QScrollArea *scrollHabilidades = new QScrollArea;
    scrollHabilidades->setWidgetResizable(true);
    scrollHabilidades->setWidget(paginaHabilidades);
    abas->addTab(scrollHabilidades, "✨ Habilidades");

    QWidget *paginaInventario = new QWidget;
    QVBoxLayout *layoutPaginaInventario = new QVBoxLayout(paginaInventario);

    // Seção própria pro dinheiro: valor atual + somar/remover uma quantia livre
    QFrame *blocoDinheiro = new QFrame;
    blocoDinheiro->setProperty("card", true);
    QHBoxLayout *linhaDinheiro = new QHBoxLayout(blocoDinheiro);
    m_dinheiroLabel = new QLabel("💰 Dinheiro: R$ 0,00");
    m_dinheiroLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    QPushButton *botaoDinheiroMais = new QPushButton("+ Adicionar");
    botaoDinheiroMais->setProperty("accent", true);
    connect(botaoDinheiroMais, &QPushButton::clicked, this, [this]() { ajustarDinheiro(true); });
    QPushButton *botaoDinheiroMenos = new QPushButton("− Remover");
    botaoDinheiroMenos->setProperty("danger", true);
    connect(botaoDinheiroMenos, &QPushButton::clicked, this, [this]() { ajustarDinheiro(false); });
    linhaDinheiro->addWidget(m_dinheiroLabel);
    linhaDinheiro->addStretch();
    linhaDinheiro->addWidget(botaoDinheiroMais);
    linhaDinheiro->addWidget(botaoDinheiroMenos);
    layoutPaginaInventario->addWidget(blocoDinheiro);
    layoutPaginaInventario->addSpacing(12);

    QHBoxLayout *cabecalhoInventario = new QHBoxLayout;
    QPushButton *botaoAddItem = new QPushButton("➕ Adicionar item");
    botaoAddItem->setProperty("accent", true);
    connect(botaoAddItem, &QPushButton::clicked, this, &TelaVisualizacao::adicionarItemInventario);
    cabecalhoInventario->addStretch();
    cabecalhoInventario->addWidget(botaoAddItem);
    layoutPaginaInventario->addLayout(cabecalhoInventario);

    m_inventarioLayout = new QVBoxLayout;
    m_inventarioLayout->setAlignment(Qt::AlignTop);
    layoutPaginaInventario->addLayout(m_inventarioLayout);

    QScrollArea *scrollInventario = new QScrollArea;
    scrollInventario->setWidgetResizable(true);
    scrollInventario->setWidget(paginaInventario);
    abas->addTab(scrollInventario, "🎒 Inventário");

    // Notas: editável direto aqui, sem precisar entrar na tela de edição —
    // negrito/itálico/alinhamento/imagens, tipo um mini editor de texto livre.
    m_notasEdit = new EditorNotas;
    connect(m_notasEdit, &EditorNotas::conteudoAlterado, this, &TelaVisualizacao::notasAlteradas);
    abas->addTab(m_notasEdit, "📝 Notas");

    corpo->addWidget(containerEsquerda, 3);
    corpo->addWidget(abas, 7);
    layoutRaiz->addLayout(corpo, 1);

    // Botão flutuante de editar (fica por cima de tudo, reposicionado no resizeEvent)
    m_botaoEditar = new QPushButton("✎", this);
    m_botaoEditar->setFixedSize(56, 56);
    m_botaoEditar->setCursor(Qt::PointingHandCursor);
    m_botaoEditar->setToolTip("Editar ficha");
    m_botaoEditar->setProperty("fab", true);
    m_botaoEditar->raise();
    connect(m_botaoEditar, &QPushButton::clicked, this, [this]() { emit editarFicha(m_caminhoArquivoAtual); });

    // Painel Lateral de Fichas Rápidas — overlay deslizante, por cima de tudo.
    m_painelFichas = new PainelLateralFichas(this);
    connect(m_painelFichas, &PainelLateralFichas::fichaSelecionada, this, &TelaVisualizacao::abrirFichaDoPainel);
    connect(m_painelFichas, &PainelLateralFichas::adicionarFichaSolicitado, this, &TelaVisualizacao::adicionarFichaAoPainel);
    m_painelFichas->raise();
}

void TelaVisualizacao::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    reposicionarBotaoEditar();
    m_painelFichas->reposicionar();
}

void TelaVisualizacao::reposicionarBotaoEditar()
{
    const int margem = 24;
    m_botaoEditar->move(width() - m_botaoEditar->width() - margem, height() - m_botaoEditar->height() - margem);
}

void TelaVisualizacao::ajustarVida(int delta)
{
    if (m_caminhoArquivoAtual.isEmpty())
        return;
    m_fichaAtual.vidaAtual = qBound(0, m_fichaAtual.vidaAtual + delta, m_fichaAtual.vidaMax);
    persistirEAtualizar();
}

void TelaVisualizacao::ajustarSanidade(int delta)
{
    if (m_caminhoArquivoAtual.isEmpty())
        return;
    m_fichaAtual.sanidadeAtual = qBound(0, m_fichaAtual.sanidadeAtual + delta, m_fichaAtual.sanidadeMax);
    persistirEAtualizar();
}

void TelaVisualizacao::ajustarDiscernimento(int delta)
{
    if (m_caminhoArquivoAtual.isEmpty())
        return;
    m_fichaAtual.discernimento = qBound(0, m_fichaAtual.discernimento + delta, 100);
    persistirEAtualizar();
}

void TelaVisualizacao::ajustarRecurso(int indice, int delta)
{
    if (m_caminhoArquivoAtual.isEmpty() || indice < 0 || indice >= m_fichaAtual.recursos.size())
        return;
    RecursoCustom &r = m_fichaAtual.recursos[indice];
    const int max = r.max > 0 ? r.max : 999999;
    r.atual = qBound(0, r.atual + delta, max);
    persistirEAtualizar();
}

void TelaVisualizacao::adicionarRecurso()
{
    if (m_caminhoArquivoAtual.isEmpty())
        return;

    QDialog dialogo(this);
    dialogo.setWindowTitle("Adicionar recurso");
    QFormLayout *form = new QFormLayout(&dialogo);

    QLineEdit *nomeEdit = new QLineEdit;
    nomeEdit->setPlaceholderText("ex: Estresse, Munição, Fadiga");
    QSpinBox *maxSpin = new QSpinBox;
    maxSpin->setRange(0, 999999);
    maxSpin->setSpecialValueText("Sem máximo (só contador)");

    form->addRow("Nome:", nomeEdit);
    form->addRow("Máximo:", maxSpin);

    QDialogButtonBox *botoes = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(botoes, &QDialogButtonBox::accepted, &dialogo, &QDialog::accept);
    connect(botoes, &QDialogButtonBox::rejected, &dialogo, &QDialog::reject);
    form->addRow(botoes);

    if (dialogo.exec() != QDialog::Accepted)
        return;
    if (nomeEdit->text().trimmed().isEmpty())
        return;

    RecursoCustom recurso;
    recurso.nome = nomeEdit->text().trimmed();
    recurso.max = maxSpin->value();
    recurso.atual = recurso.max;
    m_fichaAtual.recursos.append(recurso);

    persistirEAtualizar();
}

void TelaVisualizacao::removerRecurso(int indice)
{
    if (m_caminhoArquivoAtual.isEmpty() || indice < 0 || indice >= m_fichaAtual.recursos.size())
        return;
    if (QMessageBox::question(this, "Remover recurso", QString("Remover \"%1\"?").arg(m_fichaAtual.recursos[indice].nome)) != QMessageBox::Yes)
        return;
    m_fichaAtual.recursos.remove(indice);
    persistirEAtualizar();
}

void TelaVisualizacao::persistirEAtualizar()
{
    m_fichaAtual.salvarEmArquivo(m_caminhoArquivoAtual);
    preencherConteudo(m_fichaAtual);
    m_painelFichas->atualizarVidaFicha(m_caminhoArquivoAtual, m_fichaAtual.vidaAtual, m_fichaAtual.vidaMax);
}

void TelaVisualizacao::excluirFicha()
{
    if (m_caminhoArquivoAtual.isEmpty())
        return;

    if (Preferencias::confirmarAntesDeExcluir()) {
        const QString nomeExibido = m_fichaAtual.nome.isEmpty() ? "essa ficha" : QString("\"%1\"").arg(m_fichaAtual.nome);
        if (QMessageBox::question(this, "Excluir ficha", QString("Tem certeza que quer excluir %1? Essa ação não pode ser desfeita.").arg(nomeExibido))
            != QMessageBox::Yes)
            return;
    }

    Armazenamento::excluirFicha(m_caminhoArquivoAtual);
    PainelFichas::remover(m_caminhoArquivoAtual);
    emit fichaExcluida();
}

void TelaVisualizacao::exportarFicha()
{
    QMessageBox caixa(this);
    caixa.setWindowTitle("Exportar ficha");
    caixa.setText("Exportar em qual formato?");
    QPushButton *botaoPdf = caixa.addButton("PDF", QMessageBox::ActionRole);
    QPushButton *botaoPng = caixa.addButton("Imagem (PNG)", QMessageBox::ActionRole);
    QPushButton *botaoJson = caixa.addButton("JSON", QMessageBox::ActionRole);
    caixa.addButton(QMessageBox::Cancel);
    caixa.exec();

    QAbstractButton *escolhido = caixa.clickedButton();
    const QString nomeBase = m_fichaAtual.nome.isEmpty() ? "ficha" : m_fichaAtual.nome;

    if (escolhido == botaoPdf) {
        const QString caminho = QFileDialog::getSaveFileName(this, "Exportar como PDF", nomeBase + ".pdf", "PDF (*.pdf)");
        if (caminho.isEmpty())
            return;
        if (Exportador::exportarPdf(m_fichaAtual, caminho))
            QMessageBox::information(this, "Exportado", "Ficha exportada em PDF.");
        else
            QMessageBox::warning(this, "Erro", "Não foi possível exportar o PDF.");
    } else if (escolhido == botaoPng) {
        const QString caminho = QFileDialog::getSaveFileName(this, "Exportar como imagem", nomeBase + ".png", "Imagem PNG (*.png)");
        if (caminho.isEmpty())
            return;

        m_botaoEditar->hide();
        const bool ok = Exportador::exportarImagem(this, caminho);
        m_botaoEditar->show();

        if (ok)
            QMessageBox::information(this, "Exportado", "Ficha exportada como imagem.");
        else
            QMessageBox::warning(this, "Erro", "Não foi possível exportar a imagem.");
    } else if (escolhido == botaoJson) {
        const QString caminho = QFileDialog::getSaveFileName(this, "Exportar como JSON", nomeBase + ".json", "JSON (*.json)");
        if (caminho.isEmpty())
            return;
        if (Exportador::exportarJson(m_fichaAtual, caminho))
            QMessageBox::information(this, "Exportado", "Ficha exportada como JSON.");
        else
            QMessageBox::warning(this, "Erro", "Não foi possível exportar o JSON.");
    }
}

void TelaVisualizacao::notasAlteradas()
{
    if (m_caminhoArquivoAtual.isEmpty())
        return;
    m_fichaAtual.descricao = m_notasEdit->paraHtml();
    m_fichaAtual.salvarEmArquivo(m_caminhoArquivoAtual);
}

void TelaVisualizacao::adicionarItemInventario()
{
    if (m_caminhoArquivoAtual.isEmpty())
        return;

    QDialog dialogo(this);
    dialogo.setWindowTitle("Adicionar item");
    QFormLayout *form = new QFormLayout(&dialogo);

    QSpinBox *quantidadeSpin = new QSpinBox;
    quantidadeSpin->setRange(1, 9999);
    quantidadeSpin->setValue(1);
    QLineEdit *nomeEdit = new QLineEdit;
    nomeEdit->setPlaceholderText("Nome do item");
    QLineEdit *utilidadeEdit = new QLineEdit;
    utilidadeEdit->setPlaceholderText("Informações extras (opcional)");

    form->addRow("Quantidade:", quantidadeSpin);
    form->addRow("Nome:", nomeEdit);
    form->addRow("Informações:", utilidadeEdit);

    QDialogButtonBox *botoes = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(botoes, &QDialogButtonBox::accepted, &dialogo, &QDialog::accept);
    connect(botoes, &QDialogButtonBox::rejected, &dialogo, &QDialog::reject);
    form->addRow(botoes);

    if (dialogo.exec() != QDialog::Accepted)
        return;
    if (nomeEdit->text().trimmed().isEmpty())
        return;

    ItemInventario item;
    item.quantidade = quantidadeSpin->value();
    item.nome = nomeEdit->text().trimmed();
    item.utilidade = utilidadeEdit->text();
    m_fichaAtual.inventario.append(item);

    persistirEAtualizar();
}

void TelaVisualizacao::ajustarQuantidadeItem(int indice, int delta)
{
    if (m_caminhoArquivoAtual.isEmpty() || indice < 0 || indice >= m_fichaAtual.inventario.size())
        return;

    ItemInventario &item = m_fichaAtual.inventario[indice];
    if (item.quantidade + delta <= 0) {
        if (QMessageBox::question(this, "Remover item", QString("\"%1\" vai ficar com 0. Remover o item do inventário?").arg(item.nome))
            != QMessageBox::Yes)
            return;
        m_fichaAtual.inventario.remove(indice);
    } else {
        item.quantidade += delta;
    }

    persistirEAtualizar();
}

void TelaVisualizacao::ajustarDinheiro(bool adicionar)
{
    if (m_caminhoArquivoAtual.isEmpty())
        return;

    bool ok = false;
    const double valor = QInputDialog::getDouble(
        this, adicionar ? "Adicionar dinheiro" : "Remover dinheiro", "Valor (R$):", 0.0, 0.0, 999999999.0, 2, &ok);
    if (!ok || valor == 0.0)
        return;

    m_fichaAtual.dinheiro += adicionar ? valor : -valor;
    if (m_fichaAtual.dinheiro < 0.0)
        m_fichaAtual.dinheiro = 0.0;

    persistirEAtualizar();
}

QWidget *TelaVisualizacao::criarCardAtributo(const QString &nome, int valor, const QString &descricao)
{
    const Tema tema = GerenciadorTema::instancia().temaAtual();

    QFrame *card = new QFrame;
    card->setAttribute(Qt::WA_Hover, true);
    card->setMinimumSize(110, 90);
    card->setProperty("card", true);
    if (!descricao.trimmed().isEmpty())
        card->setToolTip(descricao);

    QVBoxLayout *layout = new QVBoxLayout(card);

    QLabel *valorLabel = new QLabel(QString::number(valor));
    valorLabel->setAlignment(Qt::AlignCenter);
    valorLabel->setStyleSheet(QString("color: %1; font-size: 28px; font-weight: bold;").arg(corParaValor(tema, valor)));

    QLabel *nomeLabel = new QLabel(nome.isEmpty() ? "—" : nome);
    nomeLabel->setAlignment(Qt::AlignCenter);
    nomeLabel->setWordWrap(true);
    nomeLabel->setStyleSheet(QString("color: %1; font-size: 12px;").arg(tema.corTextoSecundario));

    layout->addWidget(valorLabel);
    layout->addWidget(nomeLabel);
    return card;
}

QWidget *TelaVisualizacao::criarLinhaSubAtributo(const QString &nome, int valor)
{
    const Tema tema = GerenciadorTema::instancia().temaAtual();

    QFrame *linha = new QFrame;
    linha->setAttribute(Qt::WA_Hover, true);
    linha->setProperty("card", true);

    QHBoxLayout *layout = new QHBoxLayout(linha);
    QLabel *nomeLabel = new QLabel(nome);
    QLabel *valorLabel = new QLabel(QString::number(valor));
    valorLabel->setStyleSheet(QString("color: %1; font-weight: bold;").arg(corParaValor(tema, valor)));

    layout->addWidget(nomeLabel, 1);
    layout->addWidget(valorLabel);
    return linha;
}

QWidget *TelaVisualizacao::criarLinhaRecurso(int indice, const QString &nome, int atual, int max)
{
    QFrame *linha = new QFrame;
    linha->setAttribute(Qt::WA_Hover, true);
    linha->setProperty("card", true);

    QHBoxLayout *layout = new QHBoxLayout(linha);
    QPushButton *botaoMenos = criarBotaoAjuste("-");
    QLabel *label = new QLabel(max > 0 ? QString("%1: %2 / %3").arg(nome).arg(atual).arg(max) : QString("%1: %2").arg(nome).arg(atual));
    label->setStyleSheet("font-weight: bold;");
    QPushButton *botaoMais = criarBotaoAjuste("+");
    QPushButton *botaoRemover = new QPushButton("🗑");
    botaoRemover->setFixedSize(26, 26);
    botaoRemover->setProperty("compact", true);
    botaoRemover->setProperty("danger", true);

    connect(botaoMenos, &QPushButton::clicked, this, [this, indice]() { ajustarRecurso(indice, -1); });
    connect(botaoMais, &QPushButton::clicked, this, [this, indice]() { ajustarRecurso(indice, 1); });
    connect(botaoRemover, &QPushButton::clicked, this, [this, indice]() { removerRecurso(indice); });

    layout->addWidget(botaoMenos);
    layout->addWidget(label, 1);
    layout->addWidget(botaoMais);
    layout->addWidget(botaoRemover);
    return linha;
}

QWidget *TelaVisualizacao::criarCardItem(const QString &nome, const QString &descricao)
{
    const Tema tema = GerenciadorTema::instancia().temaAtual();

    QFrame *card = new QFrame;
    card->setAttribute(Qt::WA_Hover, true);
    card->setProperty("card", true);

    QVBoxLayout *layout = new QVBoxLayout(card);
    QLabel *nomeLabel = new QLabel(nome.isEmpty() ? "Sem nome" : nome);
    nomeLabel->setStyleSheet("font-weight: bold;");
    layout->addWidget(nomeLabel);

    if (!descricao.trimmed().isEmpty()) {
        QLabel *descLabel = new QLabel(descricao);
        descLabel->setWordWrap(true);
        descLabel->setStyleSheet(QString("color: %1; font-size: 12px;").arg(tema.corTextoSecundario));
        layout->addWidget(descLabel);
    }
    return card;
}

QWidget *TelaVisualizacao::criarLinhaInventario(int indice, int quantidade, const QString &nome, const QString &utilidade, bool contavel)
{
    const Tema tema = GerenciadorTema::instancia().temaAtual();

    QFrame *linha = new QFrame;
    linha->setAttribute(Qt::WA_Hover, true);
    linha->setProperty("card", true);

    QHBoxLayout *layout = new QHBoxLayout(linha);

    if (contavel) {
        QPushButton *botaoMenos = criarBotaoAjuste("-");
        connect(botaoMenos, &QPushButton::clicked, this, [this, indice]() { ajustarQuantidadeItem(indice, -1); });

        QLabel *qtdLabel = new QLabel(QString("%1x").arg(quantidade));
        qtdLabel->setFixedWidth(32);
        qtdLabel->setAlignment(Qt::AlignCenter);
        qtdLabel->setStyleSheet(QString("font-weight: bold; color: %1;").arg(tema.corAccent));

        QPushButton *botaoMais = criarBotaoAjuste("+");
        connect(botaoMais, &QPushButton::clicked, this, [this, indice]() { ajustarQuantidadeItem(indice, 1); });

        layout->addWidget(botaoMenos);
        layout->addWidget(qtdLabel);
        layout->addWidget(botaoMais);
    } else {
        layout->addSpacing(84); // mantém nome/utilidade alinhados com as linhas contáveis, sem os controles de +/-
    }

    QLabel *nomeLabel = new QLabel(nome.isEmpty() ? "Sem nome" : nome);
    nomeLabel->setStyleSheet("font-weight: bold;");
    nomeLabel->setWordWrap(true);

    QLabel *utilLabel = new QLabel(utilidade);
    utilLabel->setWordWrap(true);
    utilLabel->setStyleSheet(QString("color: %1;").arg(tema.corTextoSecundario));

    layout->addWidget(nomeLabel, 1);
    layout->addWidget(utilLabel, 2);
    return linha;
}

bool TelaVisualizacao::correspondeAoFiltro(const QString &texto) const
{
    return m_filtroTexto.trimmed().isEmpty() || texto.contains(m_filtroTexto.trimmed(), Qt::CaseInsensitive);
}

void TelaVisualizacao::carregarFicha(const CharacterSheet &ficha, const QString &caminhoArquivo)
{
    m_caminhoArquivoAtual = caminhoArquivo;
    m_fichaAtual = ficha;
    m_buscaEdit->clear(); // limpa o filtro da ficha anterior
    preencherConteudo(m_fichaAtual);

    PainelFichas::registrarAcesso(caminhoArquivo);
    m_painelFichas->atualizarLista(caminhoArquivo);

    if (!m_mostrarAoAbrirAplicado) {
        m_mostrarAoAbrirAplicado = true;
        if (PainelFichas::mostrarAoAbrir())
            m_painelFichas->alternarAberto();
    }
}

void TelaVisualizacao::alternarPainelFichas()
{
    m_painelFichas->atualizarLista(m_caminhoArquivoAtual);
    m_painelFichas->alternarAberto();
}

void TelaVisualizacao::abrirFichaDoPainel(const QString &caminho)
{
    if (caminho.isEmpty() || caminho == m_caminhoArquivoAtual)
        return;
    emit abrirOutraFicha(caminho);
}

void TelaVisualizacao::abrirNesimaFichaDoPainel(int indiceUm)
{
    const QVector<PainelFichas::EntradaAberta> entradas = PainelFichas::listarAbertas();
    const int indice = indiceUm - 1;
    if (indice < 0 || indice >= entradas.size())
        return;
    abrirFichaDoPainel(entradas[indice].caminho);
}

void TelaVisualizacao::adicionarFichaAoPainel()
{
    QStringList exibidos;
    QMap<QString, QString> paraCaminho;
    for (const QString &categoria : Armazenamento::listarCategorias()) {
        for (const QString &caminho : Armazenamento::listarArquivosFichas(categoria)) {
            CharacterSheet ficha;
            if (!CharacterSheet::carregarDeArquivo(caminho, ficha))
                continue;
            QString exibido = QString("%1 / %2").arg(categoria, ficha.nome.isEmpty() ? "Sem nome" : ficha.nome);
            while (paraCaminho.contains(exibido))
                exibido += " ";
            exibidos << exibido;
            paraCaminho[exibido] = caminho;
        }
    }

    if (exibidos.isEmpty()) {
        QMessageBox::information(this, "Adicionar ficha", "Não há nenhuma ficha criada ainda.");
        return;
    }

    bool ok = false;
    const QString escolhido = QInputDialog::getItem(this, "Adicionar ficha rápido", "Ficha:", exibidos, 0, false, &ok);
    if (!ok)
        return;

    PainelFichas::registrarAcesso(paraCaminho.value(escolhido));
    m_painelFichas->atualizarLista(m_caminhoArquivoAtual);
}

void TelaVisualizacao::preencherConteudo(const CharacterSheet &ficha)
{
    const Tema tema = GerenciadorTema::instancia().temaAtual();

    m_nomeLabel->setText(ficha.nome.isEmpty() ? "Sem nome" : ficha.nome);
    m_idadeLabel->setText(ficha.idade > 0 ? QString("%1 anos").arg(ficha.idade) : QString());
    m_alturaLabel->setText(ficha.altura);

    QString caminhoImagem;
    if (!ficha.imagemArquivo.isEmpty())
        caminhoImagem = Armazenamento::pastaImagens() + "/" + ficha.imagemArquivo;

    QPixmap pixmap;
    if (!caminhoImagem.isEmpty())
        pixmap.load(caminhoImagem);

    if (pixmap.isNull()) {
        m_imagemLabel->setPixmap(QPixmap());
        m_imagemLabel->setText("Sem imagem");
    } else {
        m_imagemLabel->setPixmap(ImagemUtil::recortarComFoco(pixmap, m_imagemLabel->size(), QPointF(ficha.imagemFocoX, ficha.imagemFocoY)));
        m_imagemLabel->setText(QString());
    }

    m_vidaLabel->setText(QString("Vida: %1 / %2").arg(ficha.vidaAtual).arg(ficha.vidaMax));
    m_vidaLabel->setStyleSheet(QString("font-size: 18px; font-weight: bold; color: %1;").arg(corParaVida(tema, ficha.vidaAtual, ficha.vidaMax)));

    m_sanidadeLabel->setText(QString("Sanidade: %1 / %2").arg(ficha.sanidadeAtual).arg(ficha.sanidadeMax));
    m_sanidadeLabel->setStyleSheet(QString("font-size: 18px; font-weight: bold; color: %1;").arg(corParaVida(tema, ficha.sanidadeAtual, ficha.sanidadeMax)));

    m_discernimentoLabel->setText(QString("Discernimento: %1%").arg(ficha.discernimento));
    m_discernimentoLabel->setStyleSheet(QString("font-size: 15px; font-weight: bold; color: %1;").arg(corParaDiscernimento(tema, ficha.discernimento)));

    // Recursos personalizados
    limparLayout(m_recursosLayout);
    for (int i = 0; i < ficha.recursos.size(); ++i) {
        const RecursoCustom &r = ficha.recursos[i];
        m_recursosLayout->addWidget(criarLinhaRecurso(i, r.nome, r.atual, r.max));
    }

    const QString semResultado = m_filtroTexto.trimmed().isEmpty() ? QString() : "Nenhum resultado para a busca.";

    // Atributos
    limparLayout(m_atributosGrid);
    const int colunas = 3;
    int linha = 0;
    int coluna = 0;
    bool temAtributo = false;
    for (const Atributo &a : ficha.atributos) {
        if (!correspondeAoFiltro(a.nome) && !correspondeAoFiltro(QString::number(a.valor)))
            continue;
        m_atributosGrid->addWidget(criarCardAtributo(a.nome, a.valor, a.descricao), linha, coluna);
        temAtributo = true;
        coluna++;
        if (coluna >= colunas) {
            coluna = 0;
            linha++;
        }
    }
    if (!temAtributo && !semResultado.isEmpty())
        m_atributosGrid->addWidget(new QLabel(semResultado), 0, 0);

    // Sub-Atributos (agrupados pelo atributo pai)
    limparLayout(m_subAtributosLayout);
    bool temSubAtributo = false;
    for (const Atributo &a : ficha.atributos) {
        if (a.subAtributos.isEmpty())
            continue;

        QVector<SubAtributo> visiveis;
        for (const SubAtributo &s : a.subAtributos) {
            if (s.nome.trimmed().isEmpty())
                continue;
            if (correspondeAoFiltro(s.nome) || correspondeAoFiltro(QString::number(s.valor)))
                visiveis << s;
        }
        if (visiveis.isEmpty())
            continue;

        QLabel *tituloGrupo = new QLabel(a.nome);
        tituloGrupo->setStyleSheet(QString("color: %1; font-weight: bold; margin-top: 8px;").arg(tema.corTextoSecundario));
        m_subAtributosLayout->addWidget(tituloGrupo);

        for (const SubAtributo &s : visiveis) {
            m_subAtributosLayout->addWidget(criarLinhaSubAtributo(s.nome, s.valor));
            temSubAtributo = true;
        }
    }
    if (!temSubAtributo)
        m_subAtributosLayout->addWidget(new QLabel(semResultado.isEmpty() ? "Nenhum sub-atributo cadastrado." : semResultado));
    m_subAtributosLayout->addStretch();

    // Habilidades (agrupadas por seção, quando houver — ex: "Rituais", "Habilidades de combate")
    limparLayout(m_habilidadesLayout);
    bool temHabilidade = false;
    {
        QStringList ordemCategorias;
        QMap<QString, QVector<Habilidade>> porCategoria;
        for (const Habilidade &hab : ficha.habilidades) {
            if (!correspondeAoFiltro(hab.nome) && !correspondeAoFiltro(hab.descricao))
                continue;
            if (!porCategoria.contains(hab.categoria))
                ordemCategorias << hab.categoria;
            porCategoria[hab.categoria].append(hab);
        }
        for (const QString &categoria : ordemCategorias) {
            if (!categoria.isEmpty()) {
                QLabel *tituloSecao = new QLabel(categoria);
                tituloSecao->setStyleSheet(QString("color: %1; font-weight: bold; margin-top: 8px;").arg(tema.corTextoSecundario));
                m_habilidadesLayout->addWidget(tituloSecao);
            }
            for (const Habilidade &hab : porCategoria.value(categoria)) {
                m_habilidadesLayout->addWidget(criarCardItem(hab.nome, hab.descricao));
                temHabilidade = true;
            }
        }
    }
    if (!temHabilidade)
        m_habilidadesLayout->addWidget(new QLabel(semResultado.isEmpty() ? "Nenhuma habilidade cadastrada." : semResultado));
    m_habilidadesLayout->addStretch();

    // Inventário
    const QLocale localeBR(QLocale::Portuguese, QLocale::Brazil);
    m_dinheiroLabel->setText("💰 Dinheiro: " + localeBR.toCurrencyString(ficha.dinheiro, "R$"));

    limparLayout(m_inventarioLayout);
    bool temItem = false;
    for (int i = 0; i < ficha.inventario.size(); ++i) {
        const ItemInventario &item = ficha.inventario[i];
        if (!correspondeAoFiltro(item.nome) && !correspondeAoFiltro(item.utilidade) && !correspondeAoFiltro(QString::number(item.quantidade)))
            continue;
        m_inventarioLayout->addWidget(criarLinhaInventario(i, item.quantidade, item.nome, item.utilidade, item.contavel));
        temItem = true;
    }
    if (!temItem)
        m_inventarioLayout->addWidget(new QLabel(semResultado.isEmpty() ? "Nenhum item no inventário." : semResultado));
    m_inventarioLayout->addStretch();

    // Notas (rich text — negrito/itálico/alinhamento/imagens)
    m_notasEdit->definirHtml(ficha.descricao);

    reposicionarBotaoEditar();
}
