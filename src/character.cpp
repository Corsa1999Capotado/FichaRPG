#include "character.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>

QJsonObject CharacterSheet::toJson() const
{
    QJsonObject obj;
    obj["nome"] = nome;
    obj["imagem"] = imagemArquivo;
    obj["idade"] = idade;
    obj["altura"] = altura;
    obj["vidaAtual"] = vidaAtual;
    obj["vidaMax"] = vidaMax;
    obj["vidaAutomatica"] = vidaAutomatica;
    obj["formulaVida"] = formulaVida;
    obj["sanidadeAtual"] = sanidadeAtual;
    obj["sanidadeMax"] = sanidadeMax;
    obj["sanidadeAutomatica"] = sanidadeAutomatica;
    obj["formulaSanidade"] = formulaSanidade;
    obj["discernimento"] = discernimento;
    obj["discernimentoAutomatico"] = discernimentoAutomatico;
    obj["formulaDiscernimento"] = formulaDiscernimento;
    obj["descricao"] = descricao;

    QJsonArray atribArray;
    for (const Atributo &atrib : atributos) {
        QJsonObject atribJson;
        atribJson["nome"] = atrib.nome;
        atribJson["valor"] = atrib.valor;
        atribJson["automatico"] = atrib.automatico;
        atribJson["formula"] = atrib.formula;

        QJsonArray subArray;
        for (const SubAtributo &sub : atrib.subAtributos) {
            QJsonObject subJson;
            subJson["nome"] = sub.nome;
            subJson["valor"] = sub.valor;
            subArray.append(subJson);
        }
        atribJson["subAtributos"] = subArray;

        atribArray.append(atribJson);
    }
    obj["atributos"] = atribArray;
    obj["dinheiro"] = dinheiro;

    QJsonArray invArray;
    for (const ItemInventario &item : inventario) {
        QJsonObject itemJson;
        itemJson["nome"] = item.nome;
        itemJson["quantidade"] = item.quantidade;
        itemJson["utilidade"] = item.utilidade;
        invArray.append(itemJson);
    }
    obj["inventario"] = invArray;

    QJsonArray habArray;
    for (const Habilidade &hab : habilidades) {
        QJsonObject habJson;
        habJson["nome"] = hab.nome;
        habJson["descricao"] = hab.descricao;
        habArray.append(habJson);
    }
    obj["habilidades"] = habArray;

    return obj;
}

CharacterSheet CharacterSheet::fromJson(const QJsonObject &obj)
{
    CharacterSheet ficha;

    ficha.nome = obj.value("nome").toString();
    ficha.imagemArquivo = obj.value("imagem").toString();
    ficha.idade = obj.value("idade").toInt();
    ficha.altura = obj.value("altura").toString();
    ficha.vidaAtual = obj.value("vidaAtual").toInt();
    ficha.vidaMax = obj.value("vidaMax").toInt();
    ficha.vidaAutomatica = obj.value("vidaAutomatica").toBool();
    ficha.formulaVida = obj.value("formulaVida").toString();
    ficha.sanidadeAtual = obj.value("sanidadeAtual").toInt();
    ficha.sanidadeMax = obj.value("sanidadeMax").toInt();
    ficha.sanidadeAutomatica = obj.value("sanidadeAutomatica").toBool();
    ficha.formulaSanidade = obj.value("formulaSanidade").toString();
    ficha.discernimento = obj.value("discernimento").toInt();
    ficha.discernimentoAutomatico = obj.value("discernimentoAutomatico").toBool();
    ficha.formulaDiscernimento = obj.value("formulaDiscernimento").toString();
    ficha.descricao = obj.value("descricao").toString();

    for (const QJsonValue &atribVal : obj.value("atributos").toArray()) {
        const QJsonObject atribJson = atribVal.toObject();
        Atributo atrib;
        atrib.nome = atribJson.value("nome").toString();
        atrib.valor = atribJson.value("valor").toInt();
        atrib.automatico = atribJson.value("automatico").toBool();
        atrib.formula = atribJson.value("formula").toString();

        for (const QJsonValue &subVal : atribJson.value("subAtributos").toArray()) {
            const QJsonObject subJson = subVal.toObject();
            SubAtributo sub;
            sub.nome = subJson.value("nome").toString();
            sub.valor = subJson.value("valor").toInt();
            atrib.subAtributos.append(sub);
        }

        ficha.atributos.append(atrib);
    }

    ficha.dinheiro = obj.value("dinheiro").toDouble();

    for (const QJsonValue &itemVal : obj.value("inventario").toArray()) {
        const QJsonObject itemJson = itemVal.toObject();
        ItemInventario item;
        item.nome = itemJson.value("nome").toString();
        item.quantidade = itemJson.value("quantidade").toInt(1);
        item.utilidade = itemJson.value("utilidade").toString();
        ficha.inventario.append(item);
    }

    for (const QJsonValue &habVal : obj.value("habilidades").toArray()) {
        const QJsonObject habJson = habVal.toObject();
        Habilidade hab;
        hab.nome = habJson.value("nome").toString();
        hab.descricao = habJson.value("descricao").toString();
        ficha.habilidades.append(hab);
    }

    return ficha;
}

bool CharacterSheet::salvarEmArquivo(const QString &caminho) const
{
    QFile arquivo(caminho);
    if (!arquivo.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    const QJsonDocument doc(toJson());
    arquivo.write(doc.toJson(QJsonDocument::Indented));
    arquivo.close();
    return true;
}

bool CharacterSheet::carregarDeArquivo(const QString &caminho, CharacterSheet &destino)
{
    QFile arquivo(caminho);
    if (!arquivo.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    const QByteArray dados = arquivo.readAll();
    arquivo.close();

    QJsonParseError erro;
    const QJsonDocument doc = QJsonDocument::fromJson(dados, &erro);
    if (erro.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    destino = CharacterSheet::fromJson(doc.object());
    return true;
}

CharacterSheet CharacterSheet::modeloArca()
{
    CharacterSheet ficha;

    ficha.atributos = {
        {"Fortitude", 0, false, QString(), {{"Força", 0}, {"Resistência", 0}, {"Luta", 0}, {"Pontaria", 0}, {"Armas Brancas", 0}}},
        {"Agilidade", 0, false, QString(), {{"Velocidade", 0}, {"Iniciativa", 0}, {"Reflexo", 0}, {"Acrobacia", 0}, {"Furtividade", 0}, {"Pilotagem", 0}}},
        {"Presença", 0, false, QString(), {{"Carisma", 0}, {"Enganar", 0}, {"Aparência", 0}, {"Liderança", 0}, {"Intimidação", 0}}},
        {"Mente", 0, false, QString(), {{"Memória", 0}, {"Ocultismo", 0}, {"Tecnologia", 0}, {"Medicina", 0}, {"Adestramento", 0}, {"Sobrevivência", 0}, {"Ciências", 0}, {"Mecânica", 0}, {"Armas de Fogo", 0}}},
        {"Percepção", 0, false, QString(), {{"Visão", 0}, {"Audição", 0}, {"Olfato", 0}, {"Intuição", 0}}},
        {"Vontade", 0, false, QString(), {{"Coragem", 0}, {"Poder", 0}, {"Conexão", 0}, {"Extra", 0}}},
    };

    ficha.idade = 0;
    ficha.altura.clear();
    ficha.vidaAtual = 0;
    ficha.vidaMax = 0;
    ficha.vidaAutomatica = true;
    ficha.formulaVida = "Fortitude * 2 + 10";
    ficha.sanidadeAtual = 0;
    ficha.sanidadeMax = 0;
    ficha.discernimento = 0;
    ficha.dinheiro = 0.0;

    ficha.descricao = "Análises:\n";

    return ficha;
}

namespace
{
bool linhaSeparadora(const QString &linha)
{
    const QString t = linha.trimmed();
    return !t.isEmpty() && t.count('=') == t.length();
}

// Procura uma seção "Habilidades:" dentro do texto "resto" (o que sobrou depois
// dos atributos), extrai cada linha "Nome - Descrição" como uma habilidade e
// remove essas linhas do texto que vai sobrar pras notas.
QVector<Habilidade> extrairHabilidades(QStringList &restoLinhas)
{
    QVector<Habilidade> habilidades;

    int inicio = -1;
    for (int i = 0; i < restoLinhas.size(); ++i) {
        const QString t = restoLinhas[i].trimmed();
        if (t.compare("Habilidades:", Qt::CaseInsensitive) == 0 || t.compare("Habilidades", Qt::CaseInsensitive) == 0) {
            inicio = i;
            break;
        }
    }
    if (inicio < 0)
        return habilidades;

    int fim = inicio + 1;
    while (fim < restoLinhas.size()) {
        const QString t = restoLinhas[fim].trimmed();
        if (t.isEmpty() || linhaSeparadora(t))
            break;

        Habilidade hab;
        const int pos = t.indexOf(" - ");
        if (pos >= 0) {
            hab.nome = t.left(pos).trimmed();
            hab.descricao = t.mid(pos + 3).trimmed();
        } else {
            hab.nome = t;
        }
        habilidades.append(hab);
        fim++;
    }

    for (int i = fim - 1; i >= inicio; --i)
        restoLinhas.removeAt(i);

    return habilidades;
}

// Fallback pra .txt que não segue o formato conhecido (sem os marcadores
// ====Atributos====/====Sub-Atributos====): tenta identificar qualquer linha
// "Rótulo: número" como um atributo solto, "Nome: ..." como o nome do
// personagem, e joga o resto nas notas.
// Tira marcadores comuns de markdown/citação ("> ", "**", "#", "-") das pontas
// da linha, pra "> **Força**: 9" virar só "Força: 9" antes de tentar casar.
QString limparMarcacaoMarkdown(QString s)
{
    static const QRegularExpression reInicio("^[>*#\\-\\s]+");
    static const QRegularExpression reFim("[*\\s]+$");
    s.replace(reInicio, "");
    s.replace(reFim, "");
    return s;
}

CharacterSheet importarGenerico(const QString &texto)
{
    static const QRegularExpression reChaveValor("^([^:=]{1,40}):\\s*(-?\\d+)\\s*$");
    static const QRegularExpression reNome("^nome\\s*:\\s*(.+)$", QRegularExpression::CaseInsensitiveOption);

    CharacterSheet ficha;
    QStringList notas;

    const QStringList linhas = texto.split('\n');
    for (const QString &linhaOriginal : linhas) {
        const QString t = limparMarcacaoMarkdown(linhaOriginal.trimmed());
        if (t.isEmpty()) {
            notas << linhaOriginal;
            continue;
        }

        const QRegularExpressionMatch mNome = reNome.match(t);
        if (ficha.nome.isEmpty() && mNome.hasMatch()) {
            ficha.nome = mNome.captured(1).trimmed();
            continue;
        }

        const QRegularExpressionMatch mChaveValor = reChaveValor.match(t);
        if (mChaveValor.hasMatch()) {
            Atributo a;
            a.nome = mChaveValor.captured(1).trimmed();
            a.valor = mChaveValor.captured(2).toInt();
            ficha.atributos.append(a);
            continue;
        }

        notas << linhaOriginal;
    }

    ficha.descricao = notas.join('\n').trimmed();
    return ficha;
}
}

CharacterSheet CharacterSheet::importarDeTexto(const QString &texto, bool *usouFormatoGenerico)
{
    enum Estado { Header, Atributos, SubAtributos, Resto };
    Estado estado = Header;
    bool marcadorAtributosEncontrado = false;

    QString nome;
    int idade = 0;
    QString altura;
    int vidaAtual = 0;
    int vidaMax = 0;
    int sanidadeAtual = 0;
    int sanidadeMax = 0;
    int discernimento = 0;
    QVector<Atributo> atributos;
    QStringList headerLinhas;
    QStringList restoLinhas;
    Atributo *atributoAtual = nullptr;

    static const QRegularExpression reVida("^vida\\s*:\\s*(\\d+)\\s*/\\s*(\\d+)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reSanidade("^sanidade\\s*:\\s*(\\d+)\\s*/\\s*(\\d+)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reIdade("^idade\\s*:\\s*(\\d+)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reAltura("^altura\\s*:\\s*(.*)$", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reDiscernimento("^discernimento\\s*:\\s*(\\d+)", QRegularExpression::CaseInsensitiveOption);

    const QStringList linhas = texto.split('\n');
    for (const QString &linhaOriginal : linhas) {
        const QString t = linhaOriginal.trimmed();

        if (estado == Header) {
            if (t.toLower().startsWith("nome") && t.contains(':')) {
                nome = t.section(':', 1).trimmed();
                continue;
            }
            QRegularExpressionMatch m = reVida.match(t);
            if (m.hasMatch()) {
                vidaAtual = m.captured(1).toInt();
                vidaMax = m.captured(2).toInt();
                continue;
            }
            m = reSanidade.match(t);
            if (m.hasMatch()) {
                sanidadeAtual = m.captured(1).toInt();
                sanidadeMax = m.captured(2).toInt();
                continue;
            }
            m = reIdade.match(t);
            if (m.hasMatch()) {
                idade = m.captured(1).toInt();
                continue;
            }
            m = reAltura.match(t);
            if (m.hasMatch()) {
                altura = m.captured(1).trimmed();
                continue;
            }
            m = reDiscernimento.match(t);
            if (m.hasMatch()) {
                discernimento = m.captured(1).toInt();
                continue;
            }
            if (t.toLower().contains("atributos") && t.contains('=') && !t.toLower().contains("sub")) {
                estado = Atributos;
                marcadorAtributosEncontrado = true;
                continue;
            }
            headerLinhas << linhaOriginal;
            continue;
        }

        if (estado == Atributos) {
            if (t.toLower().contains("sub") && t.toLower().contains("atributos") && t.contains('=')) {
                estado = SubAtributos;
                continue;
            }
            if (t.isEmpty() || !t.contains(':'))
                continue;

            Atributo a;
            a.nome = t.section(':', 0, 0).trimmed();
            a.valor = t.section(':', 1).trimmed().toInt();
            atributos.append(a);
            continue;
        }

        if (estado == SubAtributos) {
            if (linhaSeparadora(t)) {
                estado = Resto;
                continue;
            }
            if (t.isEmpty())
                continue;

            // cabeçalho de grupo: "-Nome-" (sem ':')
            if (t.startsWith('-') && t.endsWith('-') && !t.contains(':')) {
                const QString nomeGrupo = t.mid(1, t.length() - 2).trimmed();
                atributoAtual = nullptr;
                for (Atributo &a : atributos) {
                    if (a.nome.compare(nomeGrupo, Qt::CaseInsensitive) == 0) {
                        atributoAtual = &a;
                        break;
                    }
                }
                if (!atributoAtual) {
                    Atributo novo;
                    novo.nome = nomeGrupo;
                    atributos.append(novo);
                    atributoAtual = &atributos.last();
                }
                continue;
            }

            // linha de sub-atributo: "-Nome: valor"
            if (t.startsWith('-') && t.contains(':')) {
                const QString semTraco = t.mid(1);
                SubAtributo sub;
                sub.nome = semTraco.section(':', 0, 0).trimmed();
                sub.valor = semTraco.section(':', 1).trimmed().toInt();
                if (atributoAtual)
                    atributoAtual->subAtributos.append(sub);
                continue;
            }
            continue;
        }

        if (estado == Resto) {
            restoLinhas << linhaOriginal;
        }
    }

    if (!marcadorAtributosEncontrado) {
        if (usouFormatoGenerico)
            *usouFormatoGenerico = true;
        return importarGenerico(texto);
    }
    if (usouFormatoGenerico)
        *usouFormatoGenerico = false;

    const QVector<Habilidade> habilidades = extrairHabilidades(restoLinhas);

    CharacterSheet ficha;
    ficha.nome = nome;
    ficha.idade = idade;
    ficha.altura = altura;
    ficha.vidaAtual = vidaAtual;
    ficha.vidaMax = vidaMax;
    ficha.sanidadeAtual = sanidadeAtual;
    ficha.sanidadeMax = sanidadeMax;
    ficha.discernimento = discernimento;
    ficha.atributos = atributos;
    ficha.habilidades = habilidades;

    QStringList partesDescricao;
    const QString headerTexto = headerLinhas.join('\n').trimmed();
    const QString restoTexto = restoLinhas.join('\n').trimmed();
    if (!headerTexto.isEmpty())
        partesDescricao << headerTexto;
    if (!restoTexto.isEmpty())
        partesDescricao << restoTexto;
    ficha.descricao = partesDescricao.join("\n\n");

    return ficha;
}
