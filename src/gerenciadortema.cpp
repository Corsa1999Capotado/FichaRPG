#include "gerenciadortema.h"

#include <QApplication>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QStyle>
#include <QWidget>

namespace
{
QString caminhoConfig()
{
    const QString pasta = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(pasta);
    return pasta + "/config.json";
}
}

GerenciadorTema &GerenciadorTema::instancia()
{
    static GerenciadorTema unica;
    return unica;
}

GerenciadorTema::GerenciadorTema()
{
    carregarConfig();
    if (qApp)
        qApp->setStyleSheet(gerarFolhaEstilo(m_temaAtual));
}

void GerenciadorTema::carregarConfig()
{
    m_temaAtual = Tema::darkProfissional();
    m_temasCustomizados.clear();

    QFile arquivo(caminhoConfig());
    if (!arquivo.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    const QJsonDocument doc = QJsonDocument::fromJson(arquivo.readAll());
    arquivo.close();
    if (!doc.isObject())
        return;

    const QJsonObject raiz = doc.object();
    const QJsonObject temaAtivoJson = raiz.value("temaAtivo").toObject();
    if (!temaAtivoJson.isEmpty())
        m_temaAtual = Tema::fromJson(temaAtivoJson);

    for (const QJsonValue &v : raiz.value("temasCustomizados").toArray())
        m_temasCustomizados.append(Tema::fromJson(v.toObject()));
}

void GerenciadorTema::salvarConfig() const
{
    QJsonObject raiz;
    raiz["temaAtivo"] = m_temaAtual.toJson();

    QJsonArray customizados;
    for (const Tema &t : m_temasCustomizados)
        customizados.append(t.toJson());
    raiz["temasCustomizados"] = customizados;

    QFile arquivo(caminhoConfig());
    if (!arquivo.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    arquivo.write(QJsonDocument(raiz).toJson(QJsonDocument::Indented));
    arquivo.close();
}

QVector<Tema> GerenciadorTema::temasDisponiveis() const
{
    QVector<Tema> temas = {Tema::darkProfissional(), Tema::darkGamer(), Tema::lightClean()};
    temas += m_temasCustomizados;
    return temas;
}

void GerenciadorTema::aplicarTema(const Tema &tema)
{
    m_temaAtual = tema;
    salvarConfig();
    if (qApp)
        qApp->setStyleSheet(gerarFolhaEstilo(tema));
    emit temaAlterado(tema);
}

void GerenciadorTema::salvarComoTemaCustomizado(const Tema &tema)
{
    int indice = -1;
    for (int i = 0; i < m_temasCustomizados.size(); ++i) {
        if (m_temasCustomizados[i].nome.compare(tema.nome, Qt::CaseInsensitive) == 0) {
            indice = i;
            break;
        }
    }

    if (indice >= 0)
        m_temasCustomizados[indice] = tema;
    else
        m_temasCustomizados.append(tema);

    aplicarTema(tema);
}

QString GerenciadorTema::gerarFolhaEstilo(const Tema &t) const
{
    QString folha;
    folha += QString("QWidget { background-color: %1; color: %2; }").arg(t.corFundo, t.corTexto);
    folha += QString("QMainWindow, QDialog, QMessageBox { background-color: %1; }").arg(t.corFundo);
    folha += QString("QLabel { color: %1; background: transparent; }").arg(t.corTexto);
    folha += QString("QLineEdit, QTextEdit, QListWidget, QSpinBox { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 3px; }").arg(t.corCard, t.corTexto, t.corBorda);
    // Assim que um QSpinBox ganha border/padding customizados (linha acima), o Qt
    // para de calcular sozinho onde ficam os botões nativos de subir/descer — sem
    // declarar a geometria deles aqui, as setinhas ficam espremidas/sobrepostas
    // pela caixa de texto e não dá pra clicar. Só precisa reservar a área; o
    // desenho da seta em si continua vindo do estilo nativo.
    folha += QString("QSpinBox::up-button { subcontrol-origin: border; subcontrol-position: top right; width: 16px; "
                      "border-left: 1px solid %1; border-top-right-radius: 4px; }")
                 .arg(t.corBorda);
    folha += QString("QSpinBox::down-button { subcontrol-origin: border; subcontrol-position: bottom right; width: 16px; "
                      "border-left: 1px solid %1; border-bottom-right-radius: 4px; }")
                 .arg(t.corBorda);
    folha += QString("QSpinBox::up-button:hover, QSpinBox::down-button:hover { background-color: %1; }").arg(t.corCardHover);
    folha += QString("QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 6px 12px; }").arg(t.corCard, t.corTexto, t.corBorda);
    folha += QString("QPushButton:hover { background-color: %1; }").arg(t.corCardHover);
    // Botão checkable "ligado" (ex: negrito/itálico/alinhamento ativos no editor de notas)
    folha += QString("QPushButton:checked { background-color: %1; color: white; border: 1px solid %1; }").arg(t.corAccent);
    // Botões pequenos de largura/altura fixa (+/-, x, setas de mover) não têm espaço
    // pro padding padrão — sem isso o texto/ícone fica cortado e o botão parece vazio.
    folha += QString("QPushButton[compact=\"true\"] { padding: 0px; }");
    folha += QString("QPushButton[accent=\"true\"] { background-color: %1; color: white; border: none; }").arg(t.corAccent);
    folha += QString("QPushButton[accent=\"true\"]:hover { background-color: %1; }").arg(t.corAccentHover);
    folha += QString("QPushButton[fab=\"true\"] { background-color: %1; color: white; border: none; border-radius: 28px; font-size: 22px; font-weight: bold; }").arg(t.corAccent);
    folha += QString("QPushButton[fab=\"true\"]:hover { background-color: %1; }").arg(t.corAccentHover);
    folha += QString("QTabWidget::pane { border: 1px solid %1; background-color: %2; }").arg(t.corBorda, t.corFundoAlt);
    folha += QString("QTabBar::tab { background-color: %1; color: %2; padding: 8px 16px; }").arg(t.corCard, t.corTextoSecundario);
    folha += QString("QTabBar::tab:selected { background-color: %1; color: %2; }").arg(t.corFundoAlt, t.corTexto);
    folha += QString("QScrollArea { border: none; background-color: transparent; }");
    folha += QString("QFrame[card=\"true\"], QGroupBox { background-color: %1; border: 1px solid %2; border-radius: 8px; }").arg(t.corCard, t.corBorda);
    folha += QString("QFrame[card=\"true\"]:hover { background-color: %1; border: 1px solid %2; }").arg(t.corCardHover, t.corTextoSecundario);
    folha += QString("QWidget[card=\"true\"] { background-color: %1; border: 1px solid %2; border-radius: 8px; }").arg(t.corCard, t.corBorda);
    folha += QString("QWidget[card=\"true\"]:hover { border: 1px solid %1; }").arg(t.corAccent);

    // Cards de ficha no menu: hover mais chamativo (borda mais grossa + fundo mais claro)
    folha += QString("QWidget[fichaCard=\"true\"]:hover { border: 2px solid %1; background-color: %2; }").arg(t.corAccent, t.corCardHover);
    folha += QString("QCheckBox { color: %1; background: transparent; }").arg(t.corTexto);
    folha += QString("QListWidget::item:selected { background-color: %1; color: white; }").arg(t.corAccent);

    // Botões de ação destrutiva (remover) em vermelho
    folha += QString("QPushButton[danger=\"true\"] { background-color: %1; color: white; border: none; }").arg(t.corPerigo);
    folha += QString("QPushButton[danger=\"true\"]:hover { background-color: %1; }").arg(QColor(t.corPerigo).lighter(115).name());

    // Overlay vermelho translúcido em campo com erro de validação (ex: nome vazio)
    const QColor perigo(t.corPerigo);
    const QString perigoTranslucido = QString("rgba(%1, %2, %3, 70)").arg(perigo.red()).arg(perigo.green()).arg(perigo.blue());
    folha += QString("QLineEdit[invalido=\"true\"], QSpinBox[invalido=\"true\"], QTextEdit[invalido=\"true\"] { background-color: %1; border: 1px solid %2; }").arg(perigoTranslucido, t.corPerigo);

    // Campo calculado automaticamente por fórmula: itálico + fundo diferenciado
    folha += QString("QSpinBox[calculado=\"true\"] { font-style: italic; background-color: %1; border: 1px dashed %2; color: %3; }").arg(t.corFundoAlt, t.corAccent, t.corAccentHover);

    // Aviso de "alterações não salvas"
    folha += QString("QLabel[avisoAlteracao=\"true\"] { color: %1; font-weight: bold; }").arg(t.corAlerta);

    return folha;
}

void GerenciadorTema::repolir(QWidget *widget)
{
    if (!widget)
        return;
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}
