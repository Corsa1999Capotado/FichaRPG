#include "telamenu.h"

#include "armazenamento.h"
#include "cardficha.h"
#include "character.h"
#include "exportador.h"
#include "fichatemplate.h"
#include "gerenciadortema.h"
#include "gerenciarpastasdialog.h"
#include "googledrivedialog.h"
#include "painelfichas.h"
#include "preferencias.h"
#include "temadialog.h"

#include <algorithm>

#include <QAbstractButton>
#include <QAction>
#include <QActionGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QCursor>
#include <QDateTime>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QColor>
#include <QMenu>
#include <QMessageBox>
#include <QPair>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QTabBar>
#include <QTimer>
#include <QVBoxLayout>

TelaMenu::TelaMenu(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layoutRaiz = new QVBoxLayout(this);
    layoutRaiz->setContentsMargins(0, 0, 0, 0);
    layoutRaiz->setSpacing(0);

    // Cabeçalho: logo, abas de pastas, gerenciar pastas, tema/temas/templates.
    QWidget *cabecalho = new QWidget;
    cabecalho->setFixedHeight(60);
    cabecalho->setProperty("card", true);
    QHBoxLayout *layoutCabecalho = new QHBoxLayout(cabecalho);
    layoutCabecalho->setContentsMargins(16, 8, 16, 8);
    layoutCabecalho->setSpacing(10);

    QLabel *logo = new QLabel("🧙 FichaRPG");
    logo->setStyleSheet("font-size: 17px; font-weight: bold;");
    layoutCabecalho->addWidget(logo);

    layoutCabecalho->addSpacing(14);

    m_abasCategorias = new QTabBar;
    m_abasCategorias->setExpanding(false);
    m_abasCategorias->setDrawBase(false);
    connect(m_abasCategorias, &QTabBar::currentChanged, this, [this](int) { atualizarCards(); });
    layoutCabecalho->addWidget(m_abasCategorias, 1);

    QPushButton *botaoGerenciarPastas = new QPushButton("⚙️");
    botaoGerenciarPastas->setToolTip("Gerenciar pastas");
    botaoGerenciarPastas->setFixedWidth(36);
    connect(botaoGerenciarPastas, &QPushButton::clicked, this, &TelaMenu::abrirMenuPastas);
    layoutCabecalho->addWidget(botaoGerenciarPastas);

    layoutCabecalho->addStretch();

    QPushButton *botaoClaroEscuro = new QPushButton("🌗");
    botaoClaroEscuro->setToolTip("Alternar claro/escuro");
    botaoClaroEscuro->setFixedWidth(36);
    connect(botaoClaroEscuro, &QPushButton::clicked, this, &TelaMenu::alternarClaroEscuro);
    layoutCabecalho->addWidget(botaoClaroEscuro);

    QPushButton *botaoTemas = new QPushButton("🎨 Temas");
    connect(botaoTemas, &QPushButton::clicked, this, &TelaMenu::abrirDialogoTemas);
    layoutCabecalho->addWidget(botaoTemas);

    QPushButton *botaoTemplates = new QPushButton("📑 Templates");
    connect(botaoTemplates, &QPushButton::clicked, this, &TelaMenu::gerenciarTemplates);
    layoutCabecalho->addWidget(botaoTemplates);

    QPushButton *botaoPreferencias = new QPushButton("⚙️ Preferências");
    connect(botaoPreferencias, &QPushButton::clicked, this, &TelaMenu::abrirPreferencias);
    layoutCabecalho->addWidget(botaoPreferencias);

    QPushButton *botaoGoogleDrive = new QPushButton("☁️ Google Drive");
    connect(botaoGoogleDrive, &QPushButton::clicked, this, &TelaMenu::abrirGoogleDrive);
    layoutCabecalho->addWidget(botaoGoogleDrive);

    layoutRaiz->addWidget(cabecalho);

    // Busca + filtro avançado
    QWidget *linhaBusca = new QWidget;
    QHBoxLayout *layoutBusca = new QHBoxLayout(linhaBusca);
    layoutBusca->setContentsMargins(16, 10, 16, 10);
    m_buscaEdit = new QLineEdit;
    m_buscaEdit->setPlaceholderText("🔍 Buscar fichas...");
    connect(m_buscaEdit, &QLineEdit::textChanged, this, [this](const QString &texto) {
        m_filtroTexto = texto;
        atualizarCards();
    });
    layoutBusca->addWidget(m_buscaEdit, 1);

    QPushButton *botaoFiltro = new QPushButton("🔍▾");
    botaoFiltro->setToolTip("Filtro avançado");
    botaoFiltro->setFixedWidth(44);
    connect(botaoFiltro, &QPushButton::clicked, this, &TelaMenu::abrirFiltroAvancado);
    layoutBusca->addWidget(botaoFiltro);

    layoutRaiz->addWidget(linhaBusca);

    // Grid de cards da pasta selecionada
    m_containerCards = new QWidget;
    m_grid = new QGridLayout(m_containerCards);
    m_grid->setSpacing(20);
    m_grid->setContentsMargins(16, 8, 16, 24);
    m_grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(m_containerCards);
    layoutRaiz->addWidget(scroll, 1);

    // FAB flutuante "+" (filho direto da tela, reposicionado no resizeEvent)
    m_botaoNovo = new QPushButton("➕", this);
    m_botaoNovo->setFixedSize(80, 80);
    m_botaoNovo->setToolTip("Nova ficha");
    m_botaoNovo->setProperty("fab", true);
    m_botaoNovo->setCursor(Qt::PointingHandCursor);
    m_botaoNovo->setStyleSheet("border-radius: 40px; font-size: 28px;");
    connect(m_botaoNovo, &QPushButton::clicked, this, &TelaMenu::novaFicha);
    m_botaoNovo->raise();
    reposicionarBotaoNovo();

    aplicarFundoGradiente();
    connect(&GerenciadorTema::instancia(), &GerenciadorTema::temaAlterado, this, [this](const Tema &) {
        aplicarFundoGradiente();
        atualizarCards();
    });
}

void TelaMenu::aplicarFundoGradiente()
{
    const Tema tema = GerenciadorTema::instancia().temaAtual();
    const QColor topo(tema.corFundo);
    const QColor rodape = topo.darker(108);
    setStyleSheet(QString("TelaMenu { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %1, stop:1 %2); }").arg(topo.name(), rodape.name()));
}

void TelaMenu::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    reposicionarBotaoNovo();
}

void TelaMenu::reposicionarBotaoNovo()
{
    const int margem = 24;
    m_botaoNovo->move(width() - m_botaoNovo->width() - margem, height() - m_botaoNovo->height() - margem);
}

void TelaMenu::mostrarToast(const QString &mensagem, const QString &corHex)
{
    QLabel *toast = new QLabel(mensagem, this);
    toast->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    toast->setStyleSheet(QString("background-color: %1; color: white; padding: 10px 16px; border-radius: 6px; font-weight: bold;").arg(corHex));
    toast->adjustSize();
    toast->move(width() - toast->width() - 24, height() - toast->height() - 120);
    toast->show();
    toast->raise();
    QTimer::singleShot(2500, toast, &QObject::deleteLater);
}

QString TelaMenu::categoriaSelecionada() const
{
    const int indice = m_abasCategorias->currentIndex();
    return indice >= 0 ? m_abasCategorias->tabData(indice).toString() : QString();
}

void TelaMenu::abrirMenuPastas()
{
    GerenciarPastasDialog dialogo(this);
    connect(&dialogo, &GerenciarPastasDialog::pastasAlteradas, this, &TelaMenu::atualizarLista);
    dialogo.exec();
}

void TelaMenu::gerenciarTemplates()
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
        QMessageBox::information(this, "Templates", "Ainda não há nenhum template salvo.");
        return;
    }

    bool ok = false;
    const QString escolhido = QInputDialog::getItem(this, "Excluir template", "Escolha um template pra excluir:", nomesExibidos, 0, false, &ok);
    if (!ok)
        return;

    const QString caminho = nomeParaCaminho.value(escolhido);
    if (QMessageBox::question(this, "Excluir template", QString("Excluir o template \"%1\"?").arg(escolhido)) != QMessageBox::Yes)
        return;

    if (!Armazenamento::excluirTemplate(caminho))
        QMessageBox::warning(this, "Erro", "Não foi possível excluir o template.");
}

void TelaMenu::alternarClaroEscuro()
{
    const Tema atual = GerenciadorTema::instancia().temaAtual();
    GerenciadorTema::instancia().aplicarTema(atual.corFundo == Tema::lightClean().corFundo ? Tema::darkProfissional() : Tema::lightClean());
}

void TelaMenu::abrirDialogoTemas()
{
    TemaDialog dialogo(this);
    dialogo.exec();
}

void TelaMenu::abrirGoogleDrive()
{
    GoogleDriveDialog dialogo(this);
    dialogo.exec();
    atualizarLista(); // "Baixar tudo" pode ter trazido pastas/fichas novas
}

void TelaMenu::atualizarCategorias()
{
    const QString categoriaAnterior = categoriaSelecionada();

    m_abasCategorias->blockSignals(true);
    while (m_abasCategorias->count() > 0)
        m_abasCategorias->removeTab(0);

    const QStringList categorias = Armazenamento::listarCategorias();
    for (const QString &c : categorias) {
        const QString icone = Armazenamento::iconeCategoria(c);
        const int indice = m_abasCategorias->addTab(icone.isEmpty() ? c : QString("%1 %2").arg(icone, c));
        m_abasCategorias->setTabData(indice, c);
    }
    m_abasCategorias->blockSignals(false);

    int indiceParaSelecionar = categorias.indexOf(categoriaAnterior);
    if (indiceParaSelecionar < 0)
        indiceParaSelecionar = categorias.isEmpty() ? -1 : 0;

    m_abasCategorias->setCurrentIndex(indiceParaSelecionar);
    if (indiceParaSelecionar < 0)
        atualizarCards(); // não houve currentChanged (já estava em -1); limpa o grid manualmente
}

void TelaMenu::atualizarLista()
{
    atualizarCategorias();
    atualizarCards();
}

void TelaMenu::atualizarCards()
{
    for (CardFicha *card : m_cards) {
        m_grid->removeWidget(card);
        card->deleteLater();
    }
    m_cards.clear();

    const QString categoria = categoriaSelecionada();
    if (categoria.isEmpty())
        return;

    const QString filtro = m_filtroTexto.trimmed();

    struct Entrada
    {
        QString caminho;
        CharacterSheet ficha;
        QDateTime modificado;
    };
    QVector<Entrada> entradas;

    const QStringList arquivos = Armazenamento::listarArquivosFichas(categoria);
    for (const QString &caminho : arquivos) {
        CharacterSheet ficha;
        if (!CharacterSheet::carregarDeArquivo(caminho, ficha))
            continue;

        if (!filtro.isEmpty() && !ficha.nome.contains(filtro, Qt::CaseInsensitive))
            continue;

        if (m_filtroVida != FiltroVida::Todos) {
            if (ficha.vidaMax <= 0)
                continue;
            const double razao = double(ficha.vidaAtual) / double(ficha.vidaMax);
            const bool critico = razao <= 0.3;
            const bool cheio = ficha.vidaAtual >= ficha.vidaMax;
            if (m_filtroVida == FiltroVida::Critico && !critico)
                continue;
            if (m_filtroVida == FiltroVida::Cheio && !cheio)
                continue;
            if (m_filtroVida == FiltroVida::Normal && (critico || cheio))
                continue;
        }

        entradas.append({caminho, ficha, QFileInfo(caminho).lastModified()});
    }

    switch (m_ordenacao) {
    case Ordenacao::Nome:
        std::sort(entradas.begin(), entradas.end(), [](const Entrada &a, const Entrada &b) {
            return a.ficha.nome.localeAwareCompare(b.ficha.nome) < 0;
        });
        break;
    case Ordenacao::VidaMenorPrimeiro:
        std::sort(entradas.begin(), entradas.end(), [](const Entrada &a, const Entrada &b) {
            const double razaoA = a.ficha.vidaMax > 0 ? double(a.ficha.vidaAtual) / a.ficha.vidaMax : 2.0;
            const double razaoB = b.ficha.vidaMax > 0 ? double(b.ficha.vidaAtual) / b.ficha.vidaMax : 2.0;
            return razaoA < razaoB;
        });
        break;
    case Ordenacao::ModificadoRecente:
        std::sort(entradas.begin(), entradas.end(), [](const Entrada &a, const Entrada &b) {
            return a.modificado > b.modificado;
        });
        break;
    }

    const int colunas = Preferencias::colunasGrid();
    int indice = 0;
    for (const Entrada &entrada : entradas) {
        QString caminhoImagem;
        if (!entrada.ficha.imagemArquivo.isEmpty())
            caminhoImagem = Armazenamento::pastaImagens() + "/" + entrada.ficha.imagemArquivo;

        CardFicha *card = new CardFicha(entrada.caminho, entrada.ficha, caminhoImagem);
        connect(card, &CardFicha::clicado, this, &TelaMenu::abrirFicha);
        connect(card, &CardFicha::visualizarSolicitado, this, &TelaMenu::abrirFicha);
        connect(card, &CardFicha::editarSolicitado, this, &TelaMenu::editarFicha);
        connect(card, &CardFicha::duplicarSolicitado, this, &TelaMenu::duplicarFicha);
        connect(card, &CardFicha::moverSolicitado, this, &TelaMenu::moverFicha);
        connect(card, &CardFicha::exportarSolicitado, this, &TelaMenu::exportarFicha);
        connect(card, &CardFicha::excluirSolicitado, this, &TelaMenu::excluirFichaCard);

        m_grid->addWidget(card, indice / colunas, indice % colunas);
        m_cards.append(card);
        indice++;
    }
}

void TelaMenu::abrirFiltroAvancado()
{
    QMenu menu(this);

    QAction *tituloStatus = menu.addAction("Status de vida");
    tituloStatus->setEnabled(false);

    QActionGroup *grupoVida = new QActionGroup(&menu);
    grupoVida->setExclusive(true);
    const QVector<QPair<QString, FiltroVida>> opcoesVida = {
        {"Todos", FiltroVida::Todos},
        {"⚠️ Crítico (≤30%)", FiltroVida::Critico},
        {"❤️ Normal", FiltroVida::Normal},
        {"💚 Cheio (100%)", FiltroVida::Cheio},
    };
    for (const auto &opcao : opcoesVida) {
        QAction *acao = menu.addAction(opcao.first);
        acao->setCheckable(true);
        acao->setChecked(m_filtroVida == opcao.second);
        acao->setActionGroup(grupoVida);
        const FiltroVida valor = opcao.second;
        connect(acao, &QAction::triggered, this, [this, valor]() {
            m_filtroVida = valor;
            atualizarCards();
        });
    }

    menu.addSeparator();
    QAction *tituloOrdem = menu.addAction("Ordenar por");
    tituloOrdem->setEnabled(false);

    QActionGroup *grupoOrdem = new QActionGroup(&menu);
    grupoOrdem->setExclusive(true);
    const QVector<QPair<QString, Ordenacao>> opcoesOrdem = {
        {"Nome (A-Z)", Ordenacao::Nome},
        {"Vida (menor primeiro)", Ordenacao::VidaMenorPrimeiro},
        {"Modificado recentemente", Ordenacao::ModificadoRecente},
    };
    for (const auto &opcao : opcoesOrdem) {
        QAction *acao = menu.addAction(opcao.first);
        acao->setCheckable(true);
        acao->setChecked(m_ordenacao == opcao.second);
        acao->setActionGroup(grupoOrdem);
        const Ordenacao valor = opcao.second;
        connect(acao, &QAction::triggered, this, [this, valor]() {
            m_ordenacao = valor;
            atualizarCards();
        });
    }

    menu.exec(QCursor::pos());
}

void TelaMenu::abrirPreferencias()
{
    QDialog dialogo(this);
    dialogo.setWindowTitle("Preferências");
    QFormLayout *form = new QFormLayout(&dialogo);

    QComboBox *colunasCombo = new QComboBox;
    colunasCombo->addItems({"2", "3", "4"});
    colunasCombo->setCurrentText(QString::number(Preferencias::colunasGrid()));
    form->addRow("Colunas do grid:", colunasCombo);

    QCheckBox *confirmarCheck = new QCheckBox("Confirmar antes de excluir fichas/pastas");
    confirmarCheck->setChecked(Preferencias::confirmarAntesDeExcluir());
    form->addRow(confirmarCheck);

    QLabel *tituloPainel = new QLabel("📋 Painel de Fichas Rápidas");
    tituloPainel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    form->addRow(tituloPainel);

    QComboBox *larguraCombo = new QComboBox;
    larguraCombo->addItems({"200", "220", "250"});
    larguraCombo->setCurrentText(QString::number(PainelFichas::larguraPainel()));
    form->addRow("Largura do painel:", larguraCombo);

    QCheckBox *mostrarAoAbrirCheck = new QCheckBox("Mostrar painel ao abrir uma ficha");
    mostrarAoAbrirCheck->setChecked(PainelFichas::mostrarAoAbrir());
    form->addRow(mostrarAoAbrirCheck);

    QComboBox *maximoCombo = new QComboBox;
    maximoCombo->addItems({"5", "10", "15", "∞"});
    const int maximoAtual = PainelFichas::maximoFichas();
    maximoCombo->setCurrentText(maximoAtual == 0 ? "∞" : QString::number(maximoAtual));
    form->addRow("Máximo de fichas no painel:", maximoCombo);

    QCheckBox *reordenarCheck = new QCheckBox("Reordenar automaticamente (mais recentes no topo)");
    reordenarCheck->setChecked(PainelFichas::reordenarAutomatico());
    form->addRow(reordenarCheck);

    QPushButton *botaoSalvar = new QPushButton("Salvar");
    botaoSalvar->setProperty("accent", true);
    connect(botaoSalvar, &QPushButton::clicked, &dialogo, &QDialog::accept);
    form->addRow(botaoSalvar);

    if (dialogo.exec() != QDialog::Accepted)
        return;

    Preferencias::definirColunasGrid(colunasCombo->currentText().toInt());
    Preferencias::definirConfirmarAntesDeExcluir(confirmarCheck->isChecked());
    PainelFichas::definirLarguraPainel(larguraCombo->currentText().toInt());
    PainelFichas::definirMostrarAoAbrir(mostrarAoAbrirCheck->isChecked());
    PainelFichas::definirMaximoFichas(maximoCombo->currentText() == "∞" ? 0 : maximoCombo->currentText().toInt());
    PainelFichas::definirReordenarAutomatico(reordenarCheck->isChecked());
    atualizarCards();
}

void TelaMenu::duplicarFicha(const QString &caminhoArquivo)
{
    CharacterSheet ficha;
    if (!CharacterSheet::carregarDeArquivo(caminhoArquivo, ficha)) {
        QMessageBox::warning(this, "Erro", "Não foi possível abrir a ficha selecionada.");
        return;
    }

    const QString categoria = categoriaSelecionada();
    if (categoria.isEmpty())
        return;

    ficha.nome = ficha.nome.isEmpty() ? "Sem nome (cópia)" : ficha.nome + " (cópia)";

    const QString novoCaminho = Armazenamento::gerarNomeArquivoUnico(categoria, ficha.nome);
    if (!ficha.salvarEmArquivo(novoCaminho)) {
        QMessageBox::warning(this, "Erro", "Não foi possível duplicar a ficha.");
        return;
    }
    atualizarCards();
    mostrarToast("✓ Ficha duplicada", GerenciadorTema::instancia().temaAtual().corSucesso);
}

void TelaMenu::moverFicha(const QString &caminhoArquivo)
{
    const QString categoriaAtual = categoriaSelecionada();
    QStringList destinos;
    for (const QString &c : Armazenamento::listarCategorias()) {
        if (c != categoriaAtual)
            destinos << c;
    }

    if (destinos.isEmpty()) {
        QMessageBox::information(this, "Mover ficha", "Não há outra pasta pra mover essa ficha. Crie uma pasta primeiro.");
        return;
    }

    bool ok = false;
    const QString destino = QInputDialog::getItem(this, "Mover para pasta", "Pasta de destino:", destinos, 0, false, &ok);
    if (!ok || destino.isEmpty())
        return;

    CharacterSheet ficha;
    if (!CharacterSheet::carregarDeArquivo(caminhoArquivo, ficha)) {
        QMessageBox::warning(this, "Erro", "Não foi possível abrir a ficha selecionada.");
        return;
    }

    const QString novoCaminho = Armazenamento::gerarNomeArquivoUnico(destino, ficha.nome.isEmpty() ? "Sem nome" : ficha.nome);
    if (!ficha.salvarEmArquivo(novoCaminho)) {
        QMessageBox::warning(this, "Erro", "Não foi possível mover a ficha.");
        return;
    }
    Armazenamento::excluirFicha(caminhoArquivo);
    atualizarCards();
    mostrarToast(QString("✓ Ficha movida pra %1").arg(destino), GerenciadorTema::instancia().temaAtual().corSucesso);
}

void TelaMenu::exportarFicha(const QString &caminhoArquivo)
{
    CharacterSheet ficha;
    if (!CharacterSheet::carregarDeArquivo(caminhoArquivo, ficha)) {
        QMessageBox::warning(this, "Erro", "Não foi possível abrir a ficha selecionada.");
        return;
    }

    QMessageBox caixa(this);
    caixa.setWindowTitle("Exportar ficha");
    caixa.setText("Exportar em qual formato? (Pra exportar como imagem, abra a ficha e use \"Exportar\" na tela de visualização.)");
    QPushButton *botaoPdf = caixa.addButton("PDF", QMessageBox::ActionRole);
    QPushButton *botaoJson = caixa.addButton("JSON", QMessageBox::ActionRole);
    caixa.addButton(QMessageBox::Cancel);
    caixa.exec();

    QAbstractButton *escolhido = caixa.clickedButton();
    const QString nomeBase = ficha.nome.isEmpty() ? "ficha" : ficha.nome;

    if (escolhido == botaoPdf) {
        const QString caminho = QFileDialog::getSaveFileName(this, "Exportar como PDF", nomeBase + ".pdf", "PDF (*.pdf)");
        if (caminho.isEmpty())
            return;
        if (Exportador::exportarPdf(ficha, caminho))
            QMessageBox::information(this, "Exportado", "Ficha exportada em PDF.");
        else
            QMessageBox::warning(this, "Erro", "Não foi possível exportar o PDF.");
    } else if (escolhido == botaoJson) {
        const QString caminho = QFileDialog::getSaveFileName(this, "Exportar como JSON", nomeBase + ".json", "JSON (*.json)");
        if (caminho.isEmpty())
            return;
        if (Exportador::exportarJson(ficha, caminho))
            QMessageBox::information(this, "Exportado", "Ficha exportada como JSON.");
        else
            QMessageBox::warning(this, "Erro", "Não foi possível exportar o JSON.");
    }
}

void TelaMenu::excluirFichaCard(const QString &caminhoArquivo)
{
    CharacterSheet ficha;
    CharacterSheet::carregarDeArquivo(caminhoArquivo, ficha);

    if (Preferencias::confirmarAntesDeExcluir()) {
        const QString nomeExibido = ficha.nome.isEmpty() ? "essa ficha" : QString("\"%1\"").arg(ficha.nome);
        if (QMessageBox::question(this, "Excluir ficha", QString("Tem certeza que quer excluir %1? Essa ação não pode ser desfeita.").arg(nomeExibido))
            != QMessageBox::Yes)
            return;
    }

    Armazenamento::excluirFicha(caminhoArquivo);
    atualizarCards();
    mostrarToast("✓ Ficha excluída", GerenciadorTema::instancia().temaAtual().corSucesso);
}
