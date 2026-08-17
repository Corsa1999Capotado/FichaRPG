#include "cardficha.h"

#include "gerenciadortema.h"

#include <QCursor>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedLayout>
#include <QVBoxLayout>

CardFicha::CardFicha(const QString &caminhoArquivo, const CharacterSheet &ficha, const QString &caminhoImagem, QWidget *parent)
    : QWidget(parent)
    , m_caminhoArquivo(caminhoArquivo)
{
    setFixedSize(200, 280);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);
    setProperty("card", true);
    setProperty("fichaCard", true);

    const Tema tema = GerenciadorTema::instancia().temaAtual();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Foto com nome sobreposto por um gradiente escuro no rodapé (legibilidade).
    QWidget *areaFoto = new QWidget;
    areaFoto->setFixedSize(200, 150);
    QStackedLayout *pilha = new QStackedLayout(areaFoto);
    pilha->setStackingMode(QStackedLayout::StackAll);
    pilha->setContentsMargins(0, 0, 0, 0);

    QLabel *fotoLabel = new QLabel;
    fotoLabel->setFixedSize(200, 150);
    fotoLabel->setAlignment(Qt::AlignCenter);
    fotoLabel->setStyleSheet(QString("background-color: %1; border-top-left-radius: 8px; border-top-right-radius: 8px; color: %2;")
                                  .arg(tema.corFundoAlt, tema.corTextoSecundario));

    QPixmap pixmap;
    if (!caminhoImagem.isEmpty())
        pixmap.load(caminhoImagem);
    if (pixmap.isNull())
        fotoLabel->setText("Sem foto");
    else
        fotoLabel->setPixmap(pixmap.scaled(fotoLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    pilha->addWidget(fotoLabel);

    QWidget *overlayNome = new QWidget;
    overlayNome->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    overlayNome->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(0,0,0,0), stop:0.55 rgba(0,0,0,0), stop:1 rgba(0,0,0,195));");
    QVBoxLayout *layoutOverlay = new QVBoxLayout(overlayNome);
    layoutOverlay->setContentsMargins(10, 10, 10, 8);
    layoutOverlay->addStretch();
    QLabel *nomeLabel = new QLabel(ficha.nome.isEmpty() ? "Sem nome" : ficha.nome);
    nomeLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    nomeLabel->setStyleSheet("color: white; font-size: 15px; font-weight: bold; background: transparent;");
    nomeLabel->setWordWrap(true);
    layoutOverlay->addWidget(nomeLabel);
    pilha->addWidget(overlayNome);

    if (ficha.vidaMax > 0) {
        const double razao = double(ficha.vidaAtual) / double(ficha.vidaMax);
        QWidget *containerSelo = new QWidget;
        containerSelo->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        containerSelo->setStyleSheet("background: transparent;");
        QVBoxLayout *layoutSelo = new QVBoxLayout(containerSelo);
        layoutSelo->setContentsMargins(0, 0, 6, 6);
        QLabel *selo = new QLabel(razao <= 0.3 ? "⚠️" : "❤️");
        selo->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        selo->setStyleSheet("background: transparent; font-size: 16px;");
        layoutSelo->addWidget(selo, 0, Qt::AlignRight | Qt::AlignBottom);
        pilha->addWidget(containerSelo);
    }

    // Menu "⋮" de ações rápidas, sempre visível no canto superior direito da foto.
    QWidget *containerMenu = new QWidget;
    containerMenu->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    containerMenu->setStyleSheet("background: transparent;");
    QVBoxLayout *layoutMenuBtn = new QVBoxLayout(containerMenu);
    layoutMenuBtn->setContentsMargins(0, 4, 4, 0);
    QPushButton *botaoMenu = new QPushButton("⋮");
    botaoMenu->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    botaoMenu->setFixedSize(26, 26);
    botaoMenu->setCursor(Qt::PointingHandCursor);
    botaoMenu->setProperty("compact", true);
    botaoMenu->setStyleSheet("background-color: rgba(0,0,0,130); color: white; border-radius: 13px; border: none; font-weight: bold; font-size: 16px;");
    botaoMenu->setToolTip("Ações");
    connect(botaoMenu, &QPushButton::clicked, this, &CardFicha::abrirMenuAcoes);
    layoutMenuBtn->addWidget(botaoMenu, 0, Qt::AlignRight | Qt::AlignTop);
    layoutMenuBtn->addStretch();
    pilha->addWidget(containerMenu);

    layout->addWidget(areaFoto);

    // Info rápida abaixo da foto.
    QWidget *rodape = new QWidget;
    QVBoxLayout *layoutRodape = new QVBoxLayout(rodape);
    layoutRodape->setContentsMargins(10, 8, 10, 10);
    layoutRodape->setSpacing(4);

    if (ficha.idade > 0) {
        QLabel *idadeLabel = new QLabel(QString("%1 anos").arg(ficha.idade));
        idadeLabel->setStyleSheet(QString("color: %1; font-size: 12px; background: transparent;").arg(tema.corTextoSecundario));
        layoutRodape->addWidget(idadeLabel);
    }

    if (ficha.vidaMax > 0) {
        const double razao = double(ficha.vidaAtual) / double(ficha.vidaMax);
        const QString cor = razao <= 0.3 ? tema.corPerigo : tema.corSucesso;

        QLabel *vidaLabel = new QLabel(QString("❤️ %1/%2").arg(ficha.vidaAtual).arg(ficha.vidaMax));
        vidaLabel->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: bold; background: transparent;").arg(cor));
        layoutRodape->addWidget(vidaLabel);

        QProgressBar *barraVida = new QProgressBar;
        barraVida->setRange(0, ficha.vidaMax);
        barraVida->setValue(ficha.vidaAtual);
        barraVida->setTextVisible(false);
        barraVida->setFixedHeight(5);
        barraVida->setStyleSheet(QString("QProgressBar { background-color: %1; border: none; border-radius: 2px; }"
                                          "QProgressBar::chunk { background-color: %2; border-radius: 2px; }")
                                      .arg(tema.corBorda, cor));
        layoutRodape->addWidget(barraVida);
    }

    layoutRodape->addStretch();
    layout->addWidget(rodape, 1);
}

void CardFicha::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && rect().contains(event->pos()))
        emit clicado(m_caminhoArquivo);
    QWidget::mouseReleaseEvent(event);
}

void CardFicha::abrirMenuAcoes()
{
    QMenu menu(this);
    QAction *acaoEditar = menu.addAction("✎ Editar");
    QAction *acaoVisualizar = menu.addAction("👁️ Visualizar");
    QAction *acaoDuplicar = menu.addAction("📋 Duplicar");
    QAction *acaoMover = menu.addAction("🔄 Mover para pasta...");
    QAction *acaoExportar = menu.addAction("📤 Exportar");
    menu.addSeparator();
    QAction *acaoExcluir = menu.addAction("🗑️ Excluir");

    QAction *escolhida = menu.exec(QCursor::pos());
    if (escolhida == acaoEditar)
        emit editarSolicitado(m_caminhoArquivo);
    else if (escolhida == acaoVisualizar)
        emit visualizarSolicitado(m_caminhoArquivo);
    else if (escolhida == acaoDuplicar)
        emit duplicarSolicitado(m_caminhoArquivo);
    else if (escolhida == acaoMover)
        emit moverSolicitado(m_caminhoArquivo);
    else if (escolhida == acaoExportar)
        emit exportarSolicitado(m_caminhoArquivo);
    else if (escolhida == acaoExcluir)
        emit excluirSolicitado(m_caminhoArquivo);
}
