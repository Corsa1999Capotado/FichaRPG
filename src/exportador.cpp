#include "exportador.h"

#include <QFileInfo>
#include <QLocale>
#include <QRegularExpression>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QPixmap>
#include <QTextDocument>
#include <QWidget>

namespace
{
QString construirHtml(const CharacterSheet &ficha)
{
    QString html;
    html += QString("<h1>%1</h1>").arg((ficha.nome.isEmpty() ? QStringLiteral("Sem nome") : ficha.nome).toHtmlEscaped());

    const QLocale localeBR(QLocale::Portuguese, QLocale::Brazil);

    QStringList infoLinhas;
    if (ficha.idade > 0)
        infoLinhas << QString("Idade: %1").arg(ficha.idade);
    if (!ficha.altura.isEmpty())
        infoLinhas << QString("Altura: %1").arg(ficha.altura.toHtmlEscaped());
    infoLinhas << QString("Vida: %1 / %2").arg(ficha.vidaAtual).arg(ficha.vidaMax);
    infoLinhas << QString("Sanidade: %1 / %2").arg(ficha.sanidadeAtual).arg(ficha.sanidadeMax);
    infoLinhas << QString("Discernimento: %1%").arg(ficha.discernimento);
    infoLinhas << QString("Dinheiro: %1").arg(localeBR.toCurrencyString(ficha.dinheiro, "R$").toHtmlEscaped());
    html += "<p>" + infoLinhas.join("&nbsp;&nbsp;|&nbsp;&nbsp;") + "</p>";

    if (!ficha.atributos.isEmpty()) {
        html += "<h2>Atributos</h2>";
        html += "<table border=\"1\" cellspacing=\"0\" cellpadding=\"4\" width=\"100%\">";
        for (const Atributo &a : ficha.atributos) {
            html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(a.nome.toHtmlEscaped()).arg(a.valor);
            for (const SubAtributo &s : a.subAtributos) {
                if (s.nome.trimmed().isEmpty())
                    continue;
                html += QString("<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;%1</td><td>%2</td></tr>").arg(s.nome.toHtmlEscaped()).arg(s.valor);
            }
        }
        html += "</table>";
    }

    if (!ficha.inventario.isEmpty()) {
        html += "<h2>Inventário</h2><ul>";
        for (const ItemInventario &item : ficha.inventario) {
            const QString titulo = item.quantidade != 1 ? QString("%1x %2").arg(item.quantidade).arg(item.nome.toHtmlEscaped()) : item.nome.toHtmlEscaped();
            html += QString("<li><b>%1</b>%2</li>")
                        .arg(titulo, item.utilidade.trimmed().isEmpty() ? QString() : " — " + item.utilidade.toHtmlEscaped());
        }
        html += "</ul>";
    }

    if (!ficha.habilidades.isEmpty()) {
        html += "<h2>Habilidades</h2><ul>";
        for (const Habilidade &h : ficha.habilidades) {
            html += QString("<li><b>%1</b>%2</li>")
                        .arg(h.nome.toHtmlEscaped(), h.descricao.trimmed().isEmpty() ? QString() : " — " + h.descricao.toHtmlEscaped());
        }
        html += "</ul>";
    }

    if (!ficha.descricao.trimmed().isEmpty()) {
        html += "<h2>Notas</h2>";
        if (ficha.descricao.contains("<html", Qt::CaseInsensitive)) {
            // já é rich text (vem do EditorNotas) — extrai só o conteúdo do <body>,
            // já que não dá pra encaixar um documento HTML completo dentro de outro.
            static const QRegularExpression reCorpo("<body[^>]*>([\\s\\S]*)</body>", QRegularExpression::CaseInsensitiveOption);
            const QRegularExpressionMatch m = reCorpo.match(ficha.descricao);
            html += m.hasMatch() ? m.captured(1) : ficha.descricao;
        } else {
            QString notas = ficha.descricao.toHtmlEscaped();
            notas.replace("\n", "<br>");
            html += "<p>" + notas + "</p>";
        }
    }

    return html;
}
}

namespace Exportador
{

bool exportarJson(const CharacterSheet &ficha, const QString &caminho)
{
    return ficha.salvarEmArquivo(caminho);
}

bool exportarImagem(QWidget *origem, const QString &caminho)
{
    if (!origem)
        return false;
    return origem->grab().save(caminho, "PNG");
}

bool exportarPdf(const CharacterSheet &ficha, const QString &caminho)
{
    QPdfWriter writer(caminho);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(150);

    QTextDocument doc;
    doc.setHtml(construirHtml(ficha));

    const QRect pageRect = writer.pageLayout().paintRectPixels(writer.resolution());
    doc.setPageSize(QSizeF(pageRect.size()));
    doc.print(&writer);

    return QFileInfo::exists(caminho);
}

}
