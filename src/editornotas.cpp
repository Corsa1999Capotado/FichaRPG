#include "editornotas.h"

#include <functional>

#include <QBuffer>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMimeData>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWheelEvent>

namespace
{
// Procura uma imagem perto de uma posição de cursor: olha o caractere logo
// ali e, se não for imagem, o anterior (cobre tanto "em cima da imagem"
// quanto "cursor logo depois dela", que é onde ele fica após inserir uma).
bool localizarImagem(QTextDocument *doc, int posicao, QTextCursor &charCursorSaida)
{
    for (const int candidato : {posicao, posicao - 1}) {
        if (candidato < 0)
            continue;
        QTextCursor c(doc);
        c.setPosition(candidato);
        c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        if (c.charFormat().isImageFormat()) {
            charCursorSaida = c;
            return true;
        }
    }
    return false;
}

void redimensionarImagem(QTextCursor &charCursor, double fator)
{
    QTextImageFormat fmt = charCursor.charFormat().toImageFormat();
    const double larguraAtual = fmt.width() > 0 ? fmt.width() : 200.0;
    const double proporcao = (fmt.height() > 0 && fmt.width() > 0) ? fmt.height() / fmt.width() : 1.0;
    const double novaLargura = qBound(20.0, larguraAtual * fator, 2000.0);
    fmt.setWidth(novaLargura);
    fmt.setHeight(novaLargura * proporcao);
    charCursor.setCharFormat(fmt);
}

// QTextEdit especializado: roda do mouse em cima de uma imagem redimensiona
// em vez de rolar a página, e colar uma imagem do clipboard já entra com
// tamanho limitado (em vez do tamanho original, que pode ser enorme).
class AreaTexto : public QTextEdit
{
public:
    using QTextEdit::QTextEdit;
    std::function<void()> aoAlterar;

protected:
    void wheelEvent(QWheelEvent *event) override
    {
        const QTextCursor pos = cursorForPosition(event->position().toPoint());
        QTextCursor charCursor;
        if (localizarImagem(document(), pos.position(), charCursor)) {
            redimensionarImagem(charCursor, event->angleDelta().y() > 0 ? 1.1 : 0.9);
            if (aoAlterar)
                aoAlterar();
            event->accept();
            return;
        }
        QTextEdit::wheelEvent(event);
    }

    bool canInsertFromMimeData(const QMimeData *source) const override
    {
        return source->hasImage() || QTextEdit::canInsertFromMimeData(source);
    }

    void insertFromMimeData(const QMimeData *source) override
    {
        if (source->hasImage()) {
            QImage imagem = qvariant_cast<QImage>(source->imageData());
            if (!imagem.isNull()) {
                if (imagem.width() > 500)
                    imagem = imagem.scaledToWidth(500, Qt::SmoothTransformation);
                textCursor().insertImage(imagem);
                if (aoAlterar)
                    aoAlterar();
                return;
            }
        }
        QTextEdit::insertFromMimeData(source);
    }
};
}

EditorNotas::EditorNotas(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layoutRaiz = new QVBoxLayout(this);
    layoutRaiz->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *barra = new QHBoxLayout;

    m_botaoNegrito = new QPushButton("N");
    m_botaoNegrito->setCheckable(true);
    m_botaoNegrito->setFixedWidth(32);
    m_botaoNegrito->setProperty("compact", true);
    m_botaoNegrito->setStyleSheet("font-weight: 900;");
    m_botaoNegrito->setToolTip("Negrito");

    m_botaoItalico = new QPushButton("I");
    m_botaoItalico->setCheckable(true);
    m_botaoItalico->setFixedWidth(32);
    m_botaoItalico->setProperty("compact", true);
    m_botaoItalico->setStyleSheet("font-style: italic; font-size: 15px;");
    m_botaoItalico->setToolTip("Itálico");

    m_botaoEsquerda = new QPushButton("Esq.");
    m_botaoEsquerda->setCheckable(true);
    m_botaoEsquerda->setToolTip("Alinhar à esquerda");

    m_botaoCentro = new QPushButton("Centro");
    m_botaoCentro->setCheckable(true);
    m_botaoCentro->setToolTip("Centralizar");

    m_botaoDireita = new QPushButton("Dir.");
    m_botaoDireita->setCheckable(true);
    m_botaoDireita->setToolTip("Alinhar à direita");

    QPushButton *botaoImagem = new QPushButton("🖼 Imagem");
    botaoImagem->setToolTip("Insere no meio do texto, na posição do cursor (também dá pra colar com Ctrl+V)");
    connect(botaoImagem, &QPushButton::clicked, this, &EditorNotas::inserirImagem);

    QPushButton *botaoDiminuir = new QPushButton("🔍−");
    botaoDiminuir->setProperty("compact", true);
    botaoDiminuir->setFixedWidth(36);
    botaoDiminuir->setToolTip("Diminui a imagem sob/perto do cursor (ou dá roda do mouse em cima dela)");
    connect(botaoDiminuir, &QPushButton::clicked, this, [this]() { redimensionarImagemAtual(0.85); });

    QPushButton *botaoAumentar = new QPushButton("🔍+");
    botaoAumentar->setProperty("compact", true);
    botaoAumentar->setFixedWidth(36);
    botaoAumentar->setToolTip("Aumenta a imagem sob/perto do cursor (ou dá roda do mouse em cima dela)");
    connect(botaoAumentar, &QPushButton::clicked, this, [this]() { redimensionarImagemAtual(1.15); });

    barra->addWidget(m_botaoNegrito);
    barra->addWidget(m_botaoItalico);
    barra->addSpacing(10);
    barra->addWidget(m_botaoEsquerda);
    barra->addWidget(m_botaoCentro);
    barra->addWidget(m_botaoDireita);
    barra->addSpacing(10);
    barra->addWidget(botaoImagem);
    barra->addWidget(botaoDiminuir);
    barra->addWidget(botaoAumentar);
    barra->addStretch();
    layoutRaiz->addLayout(barra);

    AreaTexto *area = new AreaTexto;
    area->setPlaceholderText("Escreva livremente aqui...");
    area->setAcceptRichText(true);
    area->aoAlterar = [this]() { emit conteudoAlterado(); };
    m_texto = area;
    layoutRaiz->addWidget(m_texto);

    connect(m_botaoNegrito, &QPushButton::clicked, this, &EditorNotas::alternarNegrito);
    connect(m_botaoItalico, &QPushButton::clicked, this, &EditorNotas::alternarItalico);
    connect(m_botaoEsquerda, &QPushButton::clicked, this, [this]() { alinhar(Qt::AlignLeft); });
    connect(m_botaoCentro, &QPushButton::clicked, this, [this]() { alinhar(Qt::AlignHCenter); });
    connect(m_botaoDireita, &QPushButton::clicked, this, [this]() { alinhar(Qt::AlignRight); });

    connect(m_texto, &QTextEdit::currentCharFormatChanged, this, &EditorNotas::atualizarBotoesFormatacao);
    connect(m_texto, &QTextEdit::cursorPositionChanged, this, &EditorNotas::atualizarBotoesFormatacao);
    connect(m_texto, &QTextEdit::textChanged, this, &EditorNotas::conteudoAlterado);
}

void EditorNotas::alternarNegrito(bool ligado)
{
    QTextCharFormat fmt;
    fmt.setFontWeight(ligado ? QFont::Bold : QFont::Normal);
    m_texto->mergeCurrentCharFormat(fmt);
    m_texto->setFocus();
}

void EditorNotas::alternarItalico(bool ligado)
{
    QTextCharFormat fmt;
    fmt.setFontItalic(ligado);
    m_texto->mergeCurrentCharFormat(fmt);
    m_texto->setFocus();
}

void EditorNotas::alinhar(Qt::Alignment alinhamento)
{
    m_texto->setAlignment(alinhamento);
    m_texto->setFocus();
    atualizarBotoesFormatacao();
}

void EditorNotas::inserirImagem()
{
    const QString caminho = QFileDialog::getOpenFileName(this, "Inserir imagem", QString(), "Imagens (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (caminho.isEmpty())
        return;

    QImage imagem(caminho);
    if (imagem.isNull())
        return;

    if (imagem.width() > 500)
        imagem = imagem.scaledToWidth(500, Qt::SmoothTransformation);

    m_texto->textCursor().insertImage(imagem);
    m_texto->setFocus();
    emit conteudoAlterado();
}

void EditorNotas::redimensionarImagemAtual(double fator)
{
    QTextCursor charCursor;
    if (!localizarImagem(m_texto->document(), m_texto->textCursor().position(), charCursor)) {
        return;
    }
    redimensionarImagem(charCursor, fator);
    emit conteudoAlterado();
}

void EditorNotas::atualizarBotoesFormatacao()
{
    const QTextCharFormat fmt = m_texto->currentCharFormat();
    m_botaoNegrito->setChecked(fmt.fontWeight() >= QFont::Bold);
    m_botaoItalico->setChecked(fmt.fontItalic());

    const Qt::Alignment alinhamento = m_texto->alignment();
    m_botaoEsquerda->setChecked(alinhamento.testFlag(Qt::AlignLeft));
    m_botaoCentro->setChecked(alinhamento.testFlag(Qt::AlignHCenter));
    m_botaoDireita->setChecked(alinhamento.testFlag(Qt::AlignRight));
}

QString EditorNotas::paraHtml() const
{
    return m_texto->toHtml();
}

void EditorNotas::definirHtml(const QString &conteudo)
{
    QSignalBlocker bloqueio(m_texto);
    if (conteudo.contains("<html", Qt::CaseInsensitive)) {
        m_texto->setHtml(conteudo);
    } else {
        // texto puro (ficha antiga, ou vinda do importador de .txt) — escapa e
        // preserva quebras de linha, que em HTML não viram <br> sozinhas.
        QString escapado = conteudo.toHtmlEscaped();
        escapado.replace("\n", "<br>");
        m_texto->setHtml(escapado);
    }
    atualizarBotoesFormatacao();
}
