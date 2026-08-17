#include "telaedicao.h"

#include "armazenamento.h"
#include "editornotas.h"
#include "fichatemplate.h"
#include "formulaengine.h"
#include "gerenciadortema.h"
#include "googleauth.h"
#include "sincronizadordrive.h"

#include <QCheckBox>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

namespace
{
QString rotuloBackup(const QString &caminho)
{
    const QString base = QFileInfo(caminho).completeBaseName(); // "20260815_161530"
    const QDateTime dt = QDateTime::fromString(base, "yyyyMMdd_HHmmss");
    return dt.isValid() ? dt.toString("dd/MM/yyyy HH:mm:ss") : base;
}
}

TelaEdicao::TelaEdicao(QWidget *parent)
    : QWidget(parent)
{
    montarInterface();
}

void TelaEdicao::montarInterface()
{
    QVBoxLayout *layoutRaiz = new QVBoxLayout(this);

    // Barra superior: voltar sem salvar / título / aviso de alterações / salvar
    QHBoxLayout *barraSuperior = new QHBoxLayout;
    QPushButton *botaoVoltar = new QPushButton("← Voltar");
    QLabel *titulo = new QLabel("✏️ Editar Ficha");
    titulo->setStyleSheet("font-size: 18px; font-weight: bold;");
    m_avisoAlteracoes = new QLabel("● Alterações não salvas");
    m_avisoAlteracoes->setProperty("avisoAlteracao", true);
    m_avisoAlteracoes->setVisible(false);
    QPushButton *botaoRestaurarBackup = new QPushButton("⟲ Restaurar backup");
    QPushButton *botaoSalvarTemplate = new QPushButton("📑 Salvar como template");
    QPushButton *botaoSalvar = new QPushButton("💾 Salvar");
    botaoSalvar->setProperty("accent", true);

    barraSuperior->addWidget(botaoVoltar);
    barraSuperior->addWidget(titulo, 1, Qt::AlignCenter);
    barraSuperior->addWidget(m_avisoAlteracoes);
    barraSuperior->addWidget(botaoRestaurarBackup);
    barraSuperior->addWidget(botaoSalvarTemplate);
    barraSuperior->addWidget(botaoSalvar);
    layoutRaiz->addLayout(barraSuperior);

    connect(botaoVoltar, &QPushButton::clicked, this, [this]() {
        if (m_avisoAlteracoes->isVisible()
            && QMessageBox::question(this, "Descartar alterações?", "Você tem alterações não salvas. Sair mesmo assim?") != QMessageBox::Yes)
            return;
        emit cancelado(m_caminhoArquivoOriginal);
    });
    connect(botaoSalvar, &QPushButton::clicked, this, &TelaEdicao::salvar);
    connect(botaoSalvarTemplate, &QPushButton::clicked, this, &TelaEdicao::salvarComoTemplate);
    connect(botaoRestaurarBackup, &QPushButton::clicked, this, &TelaEdicao::restaurarBackup);

    // Conteúdo rolável
    QWidget *conteudo = new QWidget;
    QVBoxLayout *layoutConteudo = new QVBoxLayout(conteudo);

    QHBoxLayout *linhaNome = new QHBoxLayout;
    linhaNome->addWidget(new QLabel("👤 Nome:"));
    m_nomeEdit = new QLineEdit;
    m_nomeEdit->setPlaceholderText("Nome do personagem");
    linhaNome->addWidget(m_nomeEdit, 1);
    layoutConteudo->addLayout(linhaNome);

    connect(m_nomeEdit, &QLineEdit::textChanged, this, [this](const QString &texto) {
        marcarAlterado();
        if (!texto.trimmed().isEmpty() && m_nomeEdit->property("invalido").toBool()) {
            m_nomeEdit->setProperty("invalido", false);
            GerenciadorTema::repolir(m_nomeEdit);
        }
    });

    QHBoxLayout *linhaIdadeAltura = new QHBoxLayout;
    linhaIdadeAltura->addWidget(new QLabel("Idade:"));
    m_idadeSpin = new QSpinBox;
    m_idadeSpin->setRange(0, 999);
    connect(m_idadeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() { marcarAlterado(); });
    linhaIdadeAltura->addWidget(m_idadeSpin);
    linhaIdadeAltura->addWidget(new QLabel("Altura:"));
    m_alturaEdit = new QLineEdit;
    m_alturaEdit->setPlaceholderText("ex: 1,87m");
    connect(m_alturaEdit, &QLineEdit::textChanged, this, [this]() { marcarAlterado(); });
    linhaIdadeAltura->addWidget(m_alturaEdit, 1);
    layoutConteudo->addLayout(linhaIdadeAltura);

    QHBoxLayout *linhaVidaDiscernimento = new QHBoxLayout;
    linhaVidaDiscernimento->addWidget(new QLabel("❤️ Vida:"));
    m_vidaAtualSpin = new QSpinBox;
    m_vidaAtualSpin->setRange(0, 999);
    connect(m_vidaAtualSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() { marcarAlterado(); });
    m_vidaMaxSpin = new QSpinBox;
    m_vidaMaxSpin->setRange(0, 999);
    linhaVidaDiscernimento->addWidget(m_vidaAtualSpin);
    linhaVidaDiscernimento->addWidget(new QLabel("/"));
    linhaVidaDiscernimento->addWidget(m_vidaMaxSpin);
    linhaVidaDiscernimento->addSpacing(16);
    linhaVidaDiscernimento->addWidget(new QLabel("🧠 Sanidade:"));
    m_sanidadeAtualSpin = new QSpinBox;
    m_sanidadeAtualSpin->setRange(0, 999);
    connect(m_sanidadeAtualSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() { marcarAlterado(); });
    m_sanidadeMaxSpin = new QSpinBox;
    m_sanidadeMaxSpin->setRange(0, 999);
    linhaVidaDiscernimento->addWidget(m_sanidadeAtualSpin);
    linhaVidaDiscernimento->addWidget(new QLabel("/"));
    linhaVidaDiscernimento->addWidget(m_sanidadeMaxSpin);
    linhaVidaDiscernimento->addSpacing(16);
    linhaVidaDiscernimento->addWidget(new QLabel("🔮 Discernimento:"));
    m_discernimentoSpin = new QSpinBox;
    m_discernimentoSpin->setRange(0, 100);
    m_discernimentoSpin->setSuffix("%");
    linhaVidaDiscernimento->addWidget(m_discernimentoSpin);
    linhaVidaDiscernimento->addStretch();
    layoutConteudo->addLayout(linhaVidaDiscernimento);

    QHBoxLayout *linhaFormulaVida = new QHBoxLayout;
    m_vidaAutomaticaCheck = new QCheckBox("fx Vida automática:");
    m_formulaVidaEdit = new QLineEdit;
    m_formulaVidaEdit->setPlaceholderText("ex: Fortitude * 2 + 10");
    m_formulaVidaEdit->setEnabled(false);
    m_vidaMaxSpin->setEnabled(true);
    connect(m_vidaAutomaticaCheck, &QCheckBox::toggled, this, [this](bool ligado) {
        m_formulaVidaEdit->setEnabled(ligado);
        m_vidaMaxSpin->setEnabled(!ligado);
        m_vidaMaxSpin->setProperty("calculado", ligado);
        GerenciadorTema::repolir(m_vidaMaxSpin);
        marcarAlterado();
        recalcularFormulas();
    });
    connect(m_formulaVidaEdit, &QLineEdit::textChanged, this, [this]() {
        marcarAlterado();
        recalcularFormulas();
    });
    linhaFormulaVida->addWidget(m_vidaAutomaticaCheck);
    linhaFormulaVida->addWidget(m_formulaVidaEdit, 1);
    layoutConteudo->addLayout(linhaFormulaVida);

    QHBoxLayout *linhaFormulaSanidade = new QHBoxLayout;
    m_sanidadeAutomaticaCheck = new QCheckBox("fx Sanidade automática:");
    m_formulaSanidadeEdit = new QLineEdit;
    m_formulaSanidadeEdit->setPlaceholderText("ex: Vontade * 2 + 5");
    m_formulaSanidadeEdit->setEnabled(false);
    connect(m_sanidadeAutomaticaCheck, &QCheckBox::toggled, this, [this](bool ligado) {
        m_formulaSanidadeEdit->setEnabled(ligado);
        m_sanidadeMaxSpin->setEnabled(!ligado);
        m_sanidadeMaxSpin->setProperty("calculado", ligado);
        GerenciadorTema::repolir(m_sanidadeMaxSpin);
        marcarAlterado();
        recalcularFormulas();
    });
    connect(m_formulaSanidadeEdit, &QLineEdit::textChanged, this, [this]() {
        marcarAlterado();
        recalcularFormulas();
    });
    linhaFormulaSanidade->addWidget(m_sanidadeAutomaticaCheck);
    linhaFormulaSanidade->addWidget(m_formulaSanidadeEdit, 1);
    layoutConteudo->addLayout(linhaFormulaSanidade);

    QHBoxLayout *linhaFormulaDiscernimento = new QHBoxLayout;
    m_discernimentoAutomaticoCheck = new QCheckBox("fx Discernimento automático:");
    m_formulaDiscernimentoEdit = new QLineEdit;
    m_formulaDiscernimentoEdit->setPlaceholderText("ex: Mente * 5");
    m_formulaDiscernimentoEdit->setEnabled(false);
    connect(m_discernimentoAutomaticoCheck, &QCheckBox::toggled, this, [this](bool ligado) {
        m_formulaDiscernimentoEdit->setEnabled(ligado);
        m_discernimentoSpin->setEnabled(!ligado);
        m_discernimentoSpin->setProperty("calculado", ligado);
        GerenciadorTema::repolir(m_discernimentoSpin);
        marcarAlterado();
        recalcularFormulas();
    });
    connect(m_formulaDiscernimentoEdit, &QLineEdit::textChanged, this, [this]() {
        marcarAlterado();
        recalcularFormulas();
    });
    linhaFormulaDiscernimento->addWidget(m_discernimentoAutomaticoCheck);
    linhaFormulaDiscernimento->addWidget(m_formulaDiscernimentoEdit, 1);
    layoutConteudo->addLayout(linhaFormulaDiscernimento);

    QHBoxLayout *linhaImagem = new QHBoxLayout;
    m_imagemPreview = new QLabel;
    m_imagemPreview->setFixedSize(180, 180);
    m_imagemPreview->setAlignment(Qt::AlignCenter);
    m_imagemPreview->setStyleSheet("background-color: palette(midlight); border-radius: 6px;");
    m_imagemPreview->setText("Sem imagem");

    QVBoxLayout *colunaBotaoImagem = new QVBoxLayout;
    QPushButton *botaoImagem = new QPushButton("🖼️ Carregar imagem...");
    connect(botaoImagem, &QPushButton::clicked, this, &TelaEdicao::escolherImagem);
    colunaBotaoImagem->addWidget(botaoImagem);
    colunaBotaoImagem->addStretch();

    linhaImagem->addWidget(m_imagemPreview);
    linhaImagem->addLayout(colunaBotaoImagem);
    linhaImagem->addStretch();
    layoutConteudo->addLayout(linhaImagem);

    // Atributos
    QHBoxLayout *cabecalhoAtributos = new QHBoxLayout;
    QLabel *labelAtributos = new QLabel("📊 Atributos:");
    labelAtributos->setStyleSheet("font-weight: bold;");
    QPushButton *botaoAddAtributo = new QPushButton("➕ Atributo");
    botaoAddAtributo->setProperty("accent", true);
    connect(botaoAddAtributo, &QPushButton::clicked, this, [this]() {
        adicionarAtributoUI();
        marcarAlterado();
    });
    cabecalhoAtributos->addWidget(labelAtributos);
    cabecalhoAtributos->addStretch();
    cabecalhoAtributos->addWidget(botaoAddAtributo);
    layoutConteudo->addLayout(cabecalhoAtributos);

    m_atributosLayout = new QVBoxLayout;
    layoutConteudo->addLayout(m_atributosLayout);

    // Inventário
    QHBoxLayout *linhaDinheiro = new QHBoxLayout;
    linhaDinheiro->addWidget(new QLabel("💰 Dinheiro:"));
    m_dinheiroSpin = new QDoubleSpinBox;
    m_dinheiroSpin->setRange(0.0, 999999999.0);
    m_dinheiroSpin->setDecimals(2);
    m_dinheiroSpin->setPrefix("R$ ");
    connect(m_dinheiroSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { marcarAlterado(); });
    linhaDinheiro->addWidget(m_dinheiroSpin);
    linhaDinheiro->addStretch();
    layoutConteudo->addLayout(linhaDinheiro);

    QHBoxLayout *cabecalhoInventario = new QHBoxLayout;
    QLabel *labelInventario = new QLabel("🎒 Inventário:");
    labelInventario->setStyleSheet("font-weight: bold;");
    QPushButton *botaoAddItem = new QPushButton("➕ Item");
    botaoAddItem->setProperty("accent", true);
    connect(botaoAddItem, &QPushButton::clicked, this, [this]() {
        adicionarItemInventarioUI();
        marcarAlterado();
    });
    cabecalhoInventario->addWidget(labelInventario);
    cabecalhoInventario->addStretch();
    cabecalhoInventario->addWidget(botaoAddItem);
    layoutConteudo->addLayout(cabecalhoInventario);

    QHBoxLayout *legendaInventario = new QHBoxLayout;
    QLabel *legendaQtd = new QLabel("Qtd.");
    legendaQtd->setFixedWidth(48);
    QLabel *legendaNome = new QLabel("Nome");
    QLabel *legendaUtil = new QLabel("Utilidade");
    for (QLabel *l : {legendaQtd, legendaNome, legendaUtil})
        l->setStyleSheet("color: palette(mid); font-size: 11px;");
    legendaInventario->addWidget(legendaQtd);
    legendaInventario->addWidget(legendaNome, 1);
    legendaInventario->addWidget(legendaUtil, 2);
    legendaInventario->addSpacing(84); // reserva o espaço dos botões ▲▼x da linha de baixo
    layoutConteudo->addLayout(legendaInventario);

    m_inventarioLayout = new QVBoxLayout;
    layoutConteudo->addLayout(m_inventarioLayout);

    // Habilidades
    QHBoxLayout *cabecalhoHabilidades = new QHBoxLayout;
    QLabel *labelHabilidades = new QLabel("✨ Habilidades:");
    labelHabilidades->setStyleSheet("font-weight: bold;");
    QPushButton *botaoAddHabilidade = new QPushButton("➕ Habilidade");
    botaoAddHabilidade->setProperty("accent", true);
    connect(botaoAddHabilidade, &QPushButton::clicked, this, [this]() {
        adicionarLinhaNomeDescricaoUI(m_habilidadesLayout, m_habilidades, QString(), QString());
        marcarAlterado();
    });
    cabecalhoHabilidades->addWidget(labelHabilidades);
    cabecalhoHabilidades->addStretch();
    cabecalhoHabilidades->addWidget(botaoAddHabilidade);
    layoutConteudo->addLayout(cabecalhoHabilidades);

    m_habilidadesLayout = new QVBoxLayout;
    layoutConteudo->addLayout(m_habilidadesLayout);

    // Notas
    QLabel *labelNotas = new QLabel("📝 Notas:");
    labelNotas->setStyleSheet("font-weight: bold;");
    layoutConteudo->addWidget(labelNotas);
    m_descricaoEdit = new EditorNotas;
    m_descricaoEdit->setMinimumHeight(160);
    connect(m_descricaoEdit, &EditorNotas::conteudoAlterado, this, [this]() { marcarAlterado(); });
    layoutConteudo->addWidget(m_descricaoEdit);

    layoutConteudo->addStretch();

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(conteudo);
    layoutRaiz->addWidget(scroll);
}

TelaEdicao::AtributoWidgets *TelaEdicao::encontrarAtributoPorGrupo(QWidget *grupo)
{
    for (AtributoWidgets &aw : m_atributos) {
        if (aw.grupo == grupo)
            return &aw;
    }
    return nullptr;
}

void TelaEdicao::adicionarAtributoUI(const Atributo &modelo)
{
    QWidget *grupo = new QWidget;
    grupo->setObjectName("grupoAtributo");
    grupo->setStyleSheet("#grupoAtributo { border: 1px solid palette(mid); border-radius: 6px; margin-top: 4px; }");

    QVBoxLayout *layoutGrupo = new QVBoxLayout(grupo);

    QHBoxLayout *cabecalho = new QHBoxLayout;
    QLineEdit *nomeEdit = new QLineEdit(modelo.nome);
    nomeEdit->setPlaceholderText("Nome do atributo");
    QSpinBox *valorSpin = new QSpinBox;
    valorSpin->setRange(-99, 99);
    valorSpin->setValue(modelo.valor);
    valorSpin->setEnabled(!modelo.automatico);
    valorSpin->setProperty("calculado", modelo.automatico);
    QPushButton *botaoRemover = new QPushButton("🗑 Remover");
    botaoRemover->setProperty("danger", true);

    cabecalho->addWidget(nomeEdit, 1);
    cabecalho->addWidget(new QLabel("Pontos:"));
    cabecalho->addWidget(valorSpin);
    cabecalho->addWidget(botaoRemover);
    layoutGrupo->addLayout(cabecalho);

    QHBoxLayout *linhaFormula = new QHBoxLayout;
    QCheckBox *automaticoCheck = new QCheckBox("fx Fórmula automática:");
    automaticoCheck->setChecked(modelo.automatico);
    QLineEdit *formulaEdit = new QLineEdit(modelo.formula);
    formulaEdit->setPlaceholderText("ex: Agilidade + 10");
    formulaEdit->setEnabled(modelo.automatico);
    linhaFormula->addWidget(automaticoCheck);
    linhaFormula->addWidget(formulaEdit, 1);
    layoutGrupo->addLayout(linhaFormula);

    QVBoxLayout *subLayout = new QVBoxLayout;
    layoutGrupo->addLayout(subLayout);

    QPushButton *botaoAddSub = new QPushButton("➕ Sub-atributo");
    botaoAddSub->setProperty("accent", true);
    layoutGrupo->addWidget(botaoAddSub);

    AtributoWidgets widgets;
    widgets.grupo = grupo;
    widgets.nomeEdit = nomeEdit;
    widgets.valorSpin = valorSpin;
    widgets.automaticoCheck = automaticoCheck;
    widgets.formulaEdit = formulaEdit;
    widgets.subLayout = subLayout;
    m_atributos.append(widgets);

    connect(automaticoCheck, &QCheckBox::toggled, this, [this, valorSpin, formulaEdit](bool ligado) {
        valorSpin->setEnabled(!ligado);
        formulaEdit->setEnabled(ligado);
        valorSpin->setProperty("calculado", ligado);
        GerenciadorTema::repolir(valorSpin);
        marcarAlterado();
        recalcularFormulas();
    });
    connect(formulaEdit, &QLineEdit::textChanged, this, [this]() {
        marcarAlterado();
        recalcularFormulas();
    });
    connect(nomeEdit, &QLineEdit::textChanged, this, [this]() {
        marcarAlterado();
        recalcularFormulas();
    });
    connect(valorSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() {
        marcarAlterado();
        recalcularFormulas();
    });

    for (const SubAtributo &sub : modelo.subAtributos) {
        AtributoWidgets *aw = encontrarAtributoPorGrupo(grupo);
        if (aw)
            adicionarSubAtributoUI(*aw, sub);
    }

    connect(botaoRemover, &QPushButton::clicked, this, [this, grupo]() {
        removerAtributo(grupo);
        marcarAlterado();
    });
    connect(botaoAddSub, &QPushButton::clicked, this, [this, grupo]() {
        AtributoWidgets *aw = encontrarAtributoPorGrupo(grupo);
        if (aw)
            adicionarSubAtributoUI(*aw, SubAtributo());
        marcarAlterado();
    });

    m_atributosLayout->addWidget(grupo);
}

void TelaEdicao::adicionarSubAtributoUI(AtributoWidgets &atributoWidgets, const SubAtributo &modelo)
{
    QWidget *linha = new QWidget;
    QHBoxLayout *layoutLinha = new QHBoxLayout(linha);
    layoutLinha->setContentsMargins(16, 0, 0, 0);

    QLineEdit *nomeEdit = new QLineEdit(modelo.nome);
    nomeEdit->setPlaceholderText("Nome do sub-atributo");
    QSpinBox *valorSpin = new QSpinBox;
    valorSpin->setRange(-99, 99);
    valorSpin->setValue(modelo.valor);
    QPushButton *botaoMover = new QPushButton("→");
    botaoMover->setFixedWidth(28);
    botaoMover->setProperty("compact", true);
    botaoMover->setToolTip("Mover pra outro atributo");
    QPushButton *botaoRemover = new QPushButton("x");
    botaoRemover->setFixedWidth(28);
    botaoRemover->setProperty("compact", true);
    botaoRemover->setProperty("danger", true);

    layoutLinha->addWidget(nomeEdit, 1);
    layoutLinha->addWidget(valorSpin);
    layoutLinha->addWidget(botaoMover);
    layoutLinha->addWidget(botaoRemover);

    atributoWidgets.subLayout->addWidget(linha);
    atributoWidgets.subs.append({linha, nomeEdit, valorSpin});

    connect(nomeEdit, &QLineEdit::textChanged, this, [this]() { marcarAlterado(); });
    connect(valorSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() { marcarAlterado(); });

    QWidget *grupo = atributoWidgets.grupo;
    connect(botaoRemover, &QPushButton::clicked, this, [this, grupo, linha]() {
        removerSubAtributo(grupo, linha);
        marcarAlterado();
    });
    connect(botaoMover, &QPushButton::clicked, this, [this, grupo, linha]() { moverSubAtributoParaOutroGrupo(grupo, linha); });
}

void TelaEdicao::removerAtributo(QWidget *grupo)
{
    for (int i = 0; i < m_atributos.size(); ++i) {
        if (m_atributos[i].grupo == grupo) {
            m_atributosLayout->removeWidget(grupo);
            grupo->deleteLater();
            m_atributos.remove(i);
            break;
        }
    }
}

void TelaEdicao::removerSubAtributo(QWidget *grupo, QWidget *linha)
{
    AtributoWidgets *aw = encontrarAtributoPorGrupo(grupo);
    if (!aw)
        return;

    for (int i = 0; i < aw->subs.size(); ++i) {
        if (aw->subs[i].linha == linha) {
            aw->subLayout->removeWidget(linha);
            linha->deleteLater();
            aw->subs.remove(i);
            break;
        }
    }
}

void TelaEdicao::moverSubAtributoParaOutroGrupo(QWidget *grupoOrigem, QWidget *linha)
{
    AtributoWidgets *origem = encontrarAtributoPorGrupo(grupoOrigem);
    if (!origem)
        return;

    SubAtributoWidgets *encontrado = nullptr;
    for (SubAtributoWidgets &s : origem->subs) {
        if (s.linha == linha) {
            encontrado = &s;
            break;
        }
    }
    if (!encontrado)
        return;

    QStringList opcoes;
    QVector<QWidget *> gruposCorrespondentes;
    for (const AtributoWidgets &aw : m_atributos) {
        if (aw.grupo == grupoOrigem)
            continue;
        opcoes << (aw.nomeEdit->text().isEmpty() ? "(sem nome)" : aw.nomeEdit->text());
        gruposCorrespondentes << aw.grupo;
    }

    if (opcoes.isEmpty()) {
        QMessageBox::information(this, "Mover perícia", "Não há outro atributo pra mover essa perícia.");
        return;
    }

    bool ok = false;
    const QString escolhido = QInputDialog::getItem(this, "Mover perícia", "Mover pra qual atributo?", opcoes, 0, false, &ok);
    if (!ok)
        return;

    const int indiceEscolhido = opcoes.indexOf(escolhido);
    if (indiceEscolhido < 0)
        return;

    SubAtributo modelo;
    modelo.nome = encontrado->nomeEdit->text();
    modelo.valor = encontrado->valorSpin->value();

    QWidget *grupoDestino = gruposCorrespondentes[indiceEscolhido];
    removerSubAtributo(grupoOrigem, linha);

    AtributoWidgets *destino = encontrarAtributoPorGrupo(grupoDestino);
    if (destino)
        adicionarSubAtributoUI(*destino, modelo);

    marcarAlterado();
}

void TelaEdicao::limparAtributosUI()
{
    for (AtributoWidgets &aw : m_atributos) {
        m_atributosLayout->removeWidget(aw.grupo);
        aw.grupo->deleteLater();
    }
    m_atributos.clear();
}

void TelaEdicao::adicionarLinhaNomeDescricaoUI(QVBoxLayout *layout, QVector<LinhaNomeDescricaoWidgets> &lista, const QString &nome, const QString &descricao)
{
    QWidget *linha = new QWidget;
    QHBoxLayout *layoutLinha = new QHBoxLayout(linha);

    QLineEdit *nomeEdit = new QLineEdit(nome);
    nomeEdit->setPlaceholderText("Nome");
    QLineEdit *descricaoEdit = new QLineEdit(descricao);
    descricaoEdit->setPlaceholderText("Descrição (opcional)");
    QPushButton *botaoSubir = new QPushButton("▲");
    botaoSubir->setFixedWidth(24);
    botaoSubir->setProperty("compact", true);
    botaoSubir->setToolTip("Mover pra cima");
    QPushButton *botaoDescer = new QPushButton("▼");
    botaoDescer->setFixedWidth(24);
    botaoDescer->setProperty("compact", true);
    botaoDescer->setToolTip("Mover pra baixo");
    QPushButton *botaoRemover = new QPushButton("x");
    botaoRemover->setFixedWidth(28);
    botaoRemover->setProperty("compact", true);
    botaoRemover->setProperty("danger", true);

    layoutLinha->addWidget(nomeEdit, 1);
    layoutLinha->addWidget(descricaoEdit, 2);
    layoutLinha->addWidget(botaoSubir);
    layoutLinha->addWidget(botaoDescer);
    layoutLinha->addWidget(botaoRemover);

    layout->addWidget(linha);
    lista.append({linha, nomeEdit, descricaoEdit});

    connect(nomeEdit, &QLineEdit::textChanged, this, [this]() { marcarAlterado(); });
    connect(descricaoEdit, &QLineEdit::textChanged, this, [this]() { marcarAlterado(); });
    connect(botaoRemover, &QPushButton::clicked, this, [this, layout, &lista, linha]() {
        removerLinhaNomeDescricao(layout, lista, linha);
        marcarAlterado();
    });
    connect(botaoSubir, &QPushButton::clicked, this, [this, layout, &lista, linha]() {
        moverLinhaNomeDescricao(layout, lista, linha, -1);
        marcarAlterado();
    });
    connect(botaoDescer, &QPushButton::clicked, this, [this, layout, &lista, linha]() {
        moverLinhaNomeDescricao(layout, lista, linha, 1);
        marcarAlterado();
    });
}

void TelaEdicao::removerLinhaNomeDescricao(QVBoxLayout *layout, QVector<LinhaNomeDescricaoWidgets> &lista, QWidget *linha)
{
    for (int i = 0; i < lista.size(); ++i) {
        if (lista[i].linha == linha) {
            layout->removeWidget(linha);
            linha->deleteLater();
            lista.remove(i);
            break;
        }
    }
}

void TelaEdicao::moverLinhaNomeDescricao(QVBoxLayout *layout, QVector<LinhaNomeDescricaoWidgets> &lista, QWidget *linha, int direcao)
{
    int indice = -1;
    for (int i = 0; i < lista.size(); ++i) {
        if (lista[i].linha == linha) {
            indice = i;
            break;
        }
    }
    if (indice < 0)
        return;

    const int destino = indice + direcao;
    if (destino < 0 || destino >= lista.size())
        return;

    lista.swapItemsAt(indice, destino);

    const int posicaoLayout = layout->indexOf(linha);
    layout->removeWidget(linha);
    layout->insertWidget(posicaoLayout + direcao, linha);
}

void TelaEdicao::limparListaUI(QVBoxLayout *layout, QVector<LinhaNomeDescricaoWidgets> &lista)
{
    for (LinhaNomeDescricaoWidgets &w : lista) {
        layout->removeWidget(w.linha);
        w.linha->deleteLater();
    }
    lista.clear();
}

void TelaEdicao::adicionarItemInventarioUI(const ItemInventario &modelo)
{
    QWidget *linha = new QWidget;
    QHBoxLayout *layoutLinha = new QHBoxLayout(linha);

    QSpinBox *quantidadeSpin = new QSpinBox;
    quantidadeSpin->setRange(1, 9999);
    quantidadeSpin->setValue(modelo.quantidade > 0 ? modelo.quantidade : 1);
    quantidadeSpin->setFixedWidth(48);

    QLineEdit *nomeEdit = new QLineEdit(modelo.nome);
    nomeEdit->setPlaceholderText("Nome do item");
    QLineEdit *utilidadeEdit = new QLineEdit(modelo.utilidade);
    utilidadeEdit->setPlaceholderText("Pra que serve (opcional)");

    QPushButton *botaoSubir = new QPushButton("▲");
    botaoSubir->setFixedWidth(24);
    botaoSubir->setProperty("compact", true);
    botaoSubir->setToolTip("Mover pra cima");
    QPushButton *botaoDescer = new QPushButton("▼");
    botaoDescer->setFixedWidth(24);
    botaoDescer->setProperty("compact", true);
    botaoDescer->setToolTip("Mover pra baixo");
    QPushButton *botaoRemover = new QPushButton("x");
    botaoRemover->setFixedWidth(28);
    botaoRemover->setProperty("compact", true);
    botaoRemover->setProperty("danger", true);

    layoutLinha->addWidget(quantidadeSpin);
    layoutLinha->addWidget(nomeEdit, 1);
    layoutLinha->addWidget(utilidadeEdit, 2);
    layoutLinha->addWidget(botaoSubir);
    layoutLinha->addWidget(botaoDescer);
    layoutLinha->addWidget(botaoRemover);

    m_inventarioLayout->addWidget(linha);
    m_itensInventario.append({linha, quantidadeSpin, nomeEdit, utilidadeEdit});

    connect(quantidadeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() { marcarAlterado(); });
    connect(nomeEdit, &QLineEdit::textChanged, this, [this]() { marcarAlterado(); });
    connect(utilidadeEdit, &QLineEdit::textChanged, this, [this]() { marcarAlterado(); });
    connect(botaoRemover, &QPushButton::clicked, this, [this, linha]() {
        removerItemInventario(linha);
        marcarAlterado();
    });
    connect(botaoSubir, &QPushButton::clicked, this, [this, linha]() {
        moverItemInventario(linha, -1);
        marcarAlterado();
    });
    connect(botaoDescer, &QPushButton::clicked, this, [this, linha]() {
        moverItemInventario(linha, 1);
        marcarAlterado();
    });
}

void TelaEdicao::removerItemInventario(QWidget *linha)
{
    for (int i = 0; i < m_itensInventario.size(); ++i) {
        if (m_itensInventario[i].linha == linha) {
            m_inventarioLayout->removeWidget(linha);
            linha->deleteLater();
            m_itensInventario.remove(i);
            break;
        }
    }
}

void TelaEdicao::moverItemInventario(QWidget *linha, int direcao)
{
    int indice = -1;
    for (int i = 0; i < m_itensInventario.size(); ++i) {
        if (m_itensInventario[i].linha == linha) {
            indice = i;
            break;
        }
    }
    if (indice < 0)
        return;

    const int destino = indice + direcao;
    if (destino < 0 || destino >= m_itensInventario.size())
        return;

    m_itensInventario.swapItemsAt(indice, destino);

    const int posicaoLayout = m_inventarioLayout->indexOf(linha);
    m_inventarioLayout->removeWidget(linha);
    m_inventarioLayout->insertWidget(posicaoLayout + direcao, linha);
}

void TelaEdicao::limparInventarioUI()
{
    for (ItemInventarioWidgets &w : m_itensInventario) {
        m_inventarioLayout->removeWidget(w.linha);
        w.linha->deleteLater();
    }
    m_itensInventario.clear();
}

void TelaEdicao::carregarFicha(const CharacterSheet &ficha, const QString &caminhoArquivoExistente, const QString &categoriaNova)
{
    m_caminhoArquivoFicha = caminhoArquivoExistente;
    m_caminhoArquivoOriginal = caminhoArquivoExistente;
    m_categoriaNova = categoriaNova;
    m_caminhoImagemOrigemNova.clear();
    preencherCampos(ficha);
}

void TelaEdicao::preencherCampos(const CharacterSheet &ficha)
{
    m_imagemArquivoAtual = ficha.imagemArquivo;

    m_nomeEdit->setText(ficha.nome);
    m_idadeSpin->setValue(ficha.idade);
    m_alturaEdit->setText(ficha.altura);
    m_vidaAtualSpin->setValue(ficha.vidaAtual);
    m_vidaMaxSpin->setValue(ficha.vidaMax);
    m_formulaVidaEdit->setText(ficha.formulaVida);
    m_vidaAutomaticaCheck->setChecked(ficha.vidaAutomatica);
    m_sanidadeAtualSpin->setValue(ficha.sanidadeAtual);
    m_sanidadeMaxSpin->setValue(ficha.sanidadeMax);
    m_formulaSanidadeEdit->setText(ficha.formulaSanidade);
    m_sanidadeAutomaticaCheck->setChecked(ficha.sanidadeAutomatica);
    m_discernimentoSpin->setValue(ficha.discernimento);
    m_formulaDiscernimentoEdit->setText(ficha.formulaDiscernimento);
    m_discernimentoAutomaticoCheck->setChecked(ficha.discernimentoAutomatico);
    m_descricaoEdit->definirHtml(ficha.descricao);

    limparAtributosUI();
    for (const Atributo &atrib : ficha.atributos)
        adicionarAtributoUI(atrib);

    recalcularFormulas();

    m_dinheiroSpin->setValue(ficha.dinheiro);

    limparInventarioUI();
    for (const ItemInventario &item : ficha.inventario)
        adicionarItemInventarioUI(item);

    limparListaUI(m_habilidadesLayout, m_habilidades);
    for (const Habilidade &hab : ficha.habilidades)
        adicionarLinhaNomeDescricaoUI(m_habilidadesLayout, m_habilidades, hab.nome, hab.descricao);

    QString caminhoImagemPreview;
    if (!m_imagemArquivoAtual.isEmpty())
        caminhoImagemPreview = Armazenamento::pastaImagens() + "/" + m_imagemArquivoAtual;
    atualizarPreviewImagem(caminhoImagemPreview);

    if (m_nomeEdit->property("invalido").toBool()) {
        m_nomeEdit->setProperty("invalido", false);
        GerenciadorTema::repolir(m_nomeEdit);
    }
    marcarSalvo(); // ficha recém-carregada não tem alterações pendentes ainda
}

void TelaEdicao::marcarAlterado()
{
    m_avisoAlteracoes->setVisible(true);
}

void TelaEdicao::marcarSalvo()
{
    m_avisoAlteracoes->setVisible(false);
}

void TelaEdicao::escolherImagem()
{
    const QString caminho = QFileDialog::getOpenFileName(this, "Escolher imagem", QString(), "Imagens (*.png *.jpg *.jpeg *.bmp)");
    if (caminho.isEmpty())
        return;

    m_caminhoImagemOrigemNova = caminho;
    atualizarPreviewImagem(caminho);
    marcarAlterado();
}

void TelaEdicao::atualizarPreviewImagem(const QString &caminho)
{
    QPixmap pixmap;
    if (!caminho.isEmpty())
        pixmap.load(caminho);

    if (pixmap.isNull()) {
        m_imagemPreview->setPixmap(QPixmap());
        m_imagemPreview->setText("Sem imagem");
    } else {
        m_imagemPreview->setPixmap(pixmap.scaled(m_imagemPreview->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        m_imagemPreview->setText(QString());
    }
}

CharacterSheet TelaEdicao::coletarDaInterface() const
{
    CharacterSheet ficha;
    ficha.nome = m_nomeEdit->text().trimmed();
    ficha.idade = m_idadeSpin->value();
    ficha.altura = m_alturaEdit->text();
    ficha.vidaAtual = m_vidaAtualSpin->value();
    ficha.vidaMax = m_vidaMaxSpin->value();
    ficha.vidaAutomatica = m_vidaAutomaticaCheck->isChecked();
    ficha.formulaVida = m_formulaVidaEdit->text();
    ficha.sanidadeAtual = m_sanidadeAtualSpin->value();
    ficha.sanidadeMax = m_sanidadeMaxSpin->value();
    ficha.sanidadeAutomatica = m_sanidadeAutomaticaCheck->isChecked();
    ficha.formulaSanidade = m_formulaSanidadeEdit->text();
    ficha.discernimento = m_discernimentoSpin->value();
    ficha.discernimentoAutomatico = m_discernimentoAutomaticoCheck->isChecked();
    ficha.formulaDiscernimento = m_formulaDiscernimentoEdit->text();
    ficha.descricao = m_descricaoEdit->paraHtml();
    ficha.imagemArquivo = m_imagemArquivoAtual;

    for (const AtributoWidgets &aw : m_atributos) {
        Atributo atrib;
        atrib.nome = aw.nomeEdit->text();
        atrib.valor = aw.valorSpin->value();
        atrib.automatico = aw.automaticoCheck->isChecked();
        atrib.formula = aw.formulaEdit->text();

        for (const SubAtributoWidgets &sw : aw.subs) {
            SubAtributo sub;
            sub.nome = sw.nomeEdit->text();
            sub.valor = sw.valorSpin->value();
            atrib.subAtributos.append(sub);
        }
        ficha.atributos.append(atrib);
    }

    ficha.dinheiro = m_dinheiroSpin->value();

    for (const ItemInventarioWidgets &iw : m_itensInventario) {
        ItemInventario item;
        item.nome = iw.nomeEdit->text();
        item.quantidade = iw.quantidadeSpin->value();
        item.utilidade = iw.utilidadeEdit->text();
        ficha.inventario.append(item);
    }

    for (const LinhaNomeDescricaoWidgets &hw : m_habilidades) {
        Habilidade hab;
        hab.nome = hw.nomeEdit->text();
        hab.descricao = hw.descricaoEdit->text();
        ficha.habilidades.append(hab);
    }

    return ficha;
}

void TelaEdicao::recalcularFormulas()
{
    QMap<QString, double> variaveis;
    for (const AtributoWidgets &aw : m_atributos) {
        if (!aw.automaticoCheck->isChecked())
            variaveis[aw.nomeEdit->text().trimmed()] = aw.valorSpin->value();
    }

    // várias passadas pra resolver fórmulas que dependem de outras fórmulas
    const int passos = m_atributos.size() + 3;
    for (int passo = 0; passo < passos; ++passo) {
        for (const AtributoWidgets &aw : m_atributos) {
            if (!aw.automaticoCheck->isChecked())
                continue;

            bool ok = false;
            const double resultado = FormulaEngine::avaliar(aw.formulaEdit->text(), variaveis, &ok);
            if (ok) {
                aw.valorSpin->blockSignals(true);
                aw.valorSpin->setValue(qRound(resultado));
                aw.valorSpin->blockSignals(false);
                variaveis[aw.nomeEdit->text().trimmed()] = resultado;
            }
        }

        if (m_vidaAutomaticaCheck->isChecked()) {
            bool ok = false;
            const double resultado = FormulaEngine::avaliar(m_formulaVidaEdit->text(), variaveis, &ok);
            if (ok) {
                m_vidaMaxSpin->blockSignals(true);
                m_vidaMaxSpin->setValue(qRound(resultado));
                m_vidaMaxSpin->blockSignals(false);
            }
        }

        if (m_sanidadeAutomaticaCheck->isChecked()) {
            bool ok = false;
            const double resultado = FormulaEngine::avaliar(m_formulaSanidadeEdit->text(), variaveis, &ok);
            if (ok) {
                m_sanidadeMaxSpin->blockSignals(true);
                m_sanidadeMaxSpin->setValue(qRound(resultado));
                m_sanidadeMaxSpin->blockSignals(false);
            }
        }

        if (m_discernimentoAutomaticoCheck->isChecked()) {
            bool ok = false;
            const double resultado = FormulaEngine::avaliar(m_formulaDiscernimentoEdit->text(), variaveis, &ok);
            if (ok) {
                m_discernimentoSpin->blockSignals(true);
                m_discernimentoSpin->setValue(qRound(resultado));
                m_discernimentoSpin->blockSignals(false);
            }
        }
    }
}

void TelaEdicao::salvar()
{
    recalcularFormulas();
    CharacterSheet ficha = coletarDaInterface();

    if (ficha.nome.isEmpty()) {
        m_nomeEdit->setProperty("invalido", true);
        GerenciadorTema::repolir(m_nomeEdit);
        m_nomeEdit->setFocus();
        QMessageBox::warning(this, "Nome obrigatório", "Dê um nome para o personagem antes de salvar.");
        return;
    }

    if (!m_caminhoImagemOrigemNova.isEmpty()) {
        const QString nomeArquivoImagem = Armazenamento::copiarImagemParaPasta(m_caminhoImagemOrigemNova, ficha.nome);
        if (!nomeArquivoImagem.isEmpty())
            ficha.imagemArquivo = nomeArquivoImagem;
    }

    if (m_caminhoArquivoFicha.isEmpty())
        m_caminhoArquivoFicha = Armazenamento::gerarNomeArquivoUnico(m_categoriaNova, ficha.nome);
    else
        Armazenamento::salvarBackup(m_caminhoArquivoFicha); // guarda a versão anterior antes de sobrescrever

    if (!ficha.salvarEmArquivo(m_caminhoArquivoFicha)) {
        QMessageBox::warning(this, "Erro ao salvar", "Não foi possível salvar a ficha.");
        return;
    }

    const bool imagemNova = m_imagemArquivoAtual != ficha.imagemArquivo;
    m_imagemArquivoAtual = ficha.imagemArquivo;
    m_caminhoImagemOrigemNova.clear();
    marcarSalvo();

    // Com uma conta do Google conectada, sincroniza essa ficha (e a imagem
    // nova, se houver) com o Drive na hora — sem bloquear o "salvo" visual
    // pro usuário nem travar o app em caso de falha de rede.
    if (GoogleAuth::estaConectado()) {
        SincronizadorDrive::enviarArquivoUnico(m_caminhoArquivoFicha);
        if (imagemNova && !ficha.imagemArquivo.isEmpty())
            SincronizadorDrive::enviarArquivoUnico(Armazenamento::pastaImagens() + "/" + ficha.imagemArquivo);
    }

    emit salvo(m_caminhoArquivoFicha);
}

void TelaEdicao::salvarComoTemplate()
{
    bool ok = false;
    const QString nomeTemplate = QInputDialog::getText(this, "Salvar como template", "Nome do template:", QLineEdit::Normal, QString(), &ok);
    if (!ok || nomeTemplate.trimmed().isEmpty())
        return;

    FichaTemplate t;
    t.nomeTemplate = nomeTemplate.trimmed();
    t.base = coletarDaInterface();

    const QString caminho = Armazenamento::gerarNomeArquivoTemplateUnico(t.nomeTemplate);
    if (!t.salvarEmArquivo(caminho))
        QMessageBox::warning(this, "Erro", "Não foi possível salvar o template.");
    else
        QMessageBox::information(this, "Template salvo", QString("Template \"%1\" salvo com sucesso.").arg(t.nomeTemplate));
}

void TelaEdicao::restaurarBackup()
{
    if (m_caminhoArquivoOriginal.isEmpty()) {
        QMessageBox::information(this, "Sem backups", "Essa ficha ainda não foi salva, não há backups disponíveis.");
        return;
    }

    const QStringList backups = Armazenamento::listarBackups(m_caminhoArquivoOriginal);
    if (backups.isEmpty()) {
        QMessageBox::information(this, "Sem backups", "Ainda não há nenhum backup salvo pra essa ficha.");
        return;
    }

    QStringList rotulos;
    for (const QString &caminho : backups)
        rotulos << rotuloBackup(caminho);

    bool ok = false;
    const QString escolhido = QInputDialog::getItem(this, "Restaurar backup", "Escolha uma versão salva:", rotulos, 0, false, &ok);
    if (!ok)
        return;

    const int indice = rotulos.indexOf(escolhido);
    if (indice < 0)
        return;

    QByteArray jsonBytes;
    if (!Armazenamento::restaurarBackup(backups[indice], jsonBytes)) {
        QMessageBox::warning(this, "Erro", "Não foi possível ler esse backup.");
        return;
    }

    QJsonParseError erro;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &erro);
    if (erro.error != QJsonParseError::NoError || !doc.isObject()) {
        QMessageBox::warning(this, "Erro", "Esse backup está corrompido.");
        return;
    }

    if (QMessageBox::question(this, "Restaurar backup", "Isso substitui os campos atuais pelos dessa versão salva (você ainda precisa clicar em Salvar depois). Continuar?") != QMessageBox::Yes)
        return;

    preencherCampos(CharacterSheet::fromJson(doc.object()));
}
