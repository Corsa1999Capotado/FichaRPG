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
    obj["imagemFocoX"] = imagemFocoX;
    obj["imagemFocoY"] = imagemFocoY;
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
        atribJson["descricao"] = atrib.descricao;

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
        itemJson["contavel"] = item.contavel;
        invArray.append(itemJson);
    }
    obj["inventario"] = invArray;

    QJsonArray habArray;
    for (const Habilidade &hab : habilidades) {
        QJsonObject habJson;
        habJson["nome"] = hab.nome;
        habJson["descricao"] = hab.descricao;
        habJson["categoria"] = hab.categoria;
        habArray.append(habJson);
    }
    obj["habilidades"] = habArray;

    QJsonArray recArray;
    for (const RecursoCustom &r : recursos) {
        QJsonObject recJson;
        recJson["nome"] = r.nome;
        recJson["atual"] = r.atual;
        recJson["max"] = r.max;
        recArray.append(recJson);
    }
    obj["recursos"] = recArray;

    return obj;
}

CharacterSheet CharacterSheet::fromJson(const QJsonObject &obj)
{
    CharacterSheet ficha;

    ficha.nome = obj.value("nome").toString();
    ficha.imagemArquivo = obj.value("imagem").toString();
    ficha.imagemFocoX = obj.value("imagemFocoX").toDouble(0.5);
    ficha.imagemFocoY = obj.value("imagemFocoY").toDouble(0.5);
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
        atrib.descricao = atribJson.value("descricao").toString();

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
        item.contavel = itemJson.value("contavel").toBool(true);
        ficha.inventario.append(item);
    }

    for (const QJsonValue &habVal : obj.value("habilidades").toArray()) {
        const QJsonObject habJson = habVal.toObject();
        Habilidade hab;
        hab.nome = habJson.value("nome").toString();
        hab.descricao = habJson.value("descricao").toString();
        hab.categoria = habJson.value("categoria").toString();
        ficha.habilidades.append(hab);
    }

    for (const QJsonValue &recVal : obj.value("recursos").toArray()) {
        const QJsonObject recJson = recVal.toObject();
        RecursoCustom r;
        r.nome = recJson.value("nome").toString();
        r.atual = recJson.value("atual").toInt();
        r.max = recJson.value("max").toInt();
        ficha.recursos.append(r);
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
        {"Fortitude", 0, false, QString(), QString(), {{"Força", 0}, {"Resistência", 0}, {"Luta", 0}, {"Pontaria", 0}, {"Armas Brancas", 0}}},
        {"Agilidade", 0, false, QString(), QString(), {{"Velocidade", 0}, {"Iniciativa", 0}, {"Reflexo", 0}, {"Acrobacia", 0}, {"Furtividade", 0}, {"Pilotagem", 0}}},
        {"Presença", 0, false, QString(), QString(), {{"Carisma", 0}, {"Enganar", 0}, {"Aparência", 0}, {"Liderança", 0}, {"Intimidação", 0}}},
        {"Mente", 0, false, QString(), QString(), {{"Memória", 0}, {"Ocultismo", 0}, {"Tecnologia", 0}, {"Medicina", 0}, {"Adestramento", 0}, {"Sobrevivência", 0}, {"Ciências", 0}, {"Mecânica", 0}, {"Armas de Fogo", 0}}},
        {"Percepção", 0, false, QString(), QString(), {{"Visão", 0}, {"Audição", 0}, {"Olfato", 0}, {"Intuição", 0}}},
        {"Vontade", 0, false, QString(), QString(), {{"Coragem", 0}, {"Poder", 0}, {"Conexão", 0}, {"Extra", 0}}},
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

// Linha decorativa feita só de '-'/'=' (ex. "----------===========-----"),
// usada por alguns modelos pra separar seções soltas fora do padrão
// "====Nome====" — mais permissiva que linhaSeparadora (que exige só '=').
bool linhaDivisoria(const QString &linha)
{
    const QString t = linha.trimmed();
    if (t.length() < 4)
        return false;
    for (const QChar &c : t) {
        if (c != '-' && c != '=')
            return false;
    }
    return true;
}

// Alguns .txt chegam com mojibake: o arquivo original era UTF-8, mas foi lido
// como Latin-1/CP1252 em algum passo (editor, copiar-colar) e regravado como
// UTF-8 — daí "não" virar "nÃ£o". Detecta esse padrão (sequência "Ã" seguida
// de outro caractere Latin-1 típico de continuação UTF-8) e desfaz com um
// round-trip fromUtf8(toLatin1()). Só aplica a correção se ela reduzir as
// ocorrências suspeitas, pra não estragar texto que já estava correto.
QString corrigirMojibake(const QString &texto)
{
    static const QRegularExpression reSuspeita("Ã[\\x{0080}-\\x{00BF}]");

    const int suspeitasAntes = texto.count(reSuspeita);
    if (suspeitasAntes == 0)
        return texto;

    const QByteArray comoLatin1 = texto.toLatin1();
    if (comoLatin1.isEmpty())
        return texto;

    const QString corrigido = QString::fromUtf8(comoLatin1);
    if (corrigido.isEmpty())
        return texto;

    const int suspeitasDepois = corrigido.count(reSuspeita);
    return suspeitasDepois < suspeitasAntes ? corrigido : texto;
}

// Tira marcadores de markdown (negrito "**"/"*"/"_", ":" do fim) das pontas
// de um marcador de seção, deixando só o texto pra comparar (ex: "*Inventario:*"
// vira "inventario"). Usado pra reconhecer cabeçalhos de seção em modelos que
// escrevem os marcadores em negrito em vez do "====Nome====" tradicional.
QString normalizarMarcador(QString t)
{
    t = t.trimmed();
    static const QRegularExpression rePontas("^[*_\\s]+|[*_\\s:]+$");
    t.replace(rePontas, "");
    return t.trimmed().toLower();
}

// Tira negrito/itálico markdown ("**Nome**" -> "Nome") de uma entrada solta
// (nome de habilidade/item), sem mexer no meio do texto.
QString limparNegrito(QString s)
{
    s = s.trimmed();
    static const QRegularExpression rePontas("^\\*+|\\*+$");
    s.replace(rePontas, "");
    return s.trimmed();
}

// Procura uma seção "<marcador>:" (ex. "Habilidades:", "*Rituais:*") dentro do
// texto "resto", extrai cada linha "Nome - Descrição" ou "Nome : Descrição"
// como uma habilidade e remove essas linhas do texto que vai sobrar pras notas.
QVector<Habilidade> extrairHabilidades(QStringList &restoLinhas, const QString &marcador)
{
    QVector<Habilidade> habilidades;

    int inicio = -1;
    for (int i = 0; i < restoLinhas.size(); ++i) {
        if (normalizarMarcador(restoLinhas[i]) == marcador.toLower()) {
            inicio = i;
            break;
        }
    }
    if (inicio < 0)
        return habilidades;

    int fim = inicio + 1;
    while (fim < restoLinhas.size()) {
        const QString t = restoLinhas[fim].trimmed();
        if (t.isEmpty() || linhaDivisoria(t))
            break;

        Habilidade hab;
        const int posTraco = t.indexOf(" - ");
        const int posDoisPontos = t.indexOf(" : ");
        if (posDoisPontos >= 0 && (posTraco < 0 || posDoisPontos < posTraco)) {
            hab.nome = limparNegrito(t.left(posDoisPontos));
            hab.descricao = t.mid(posDoisPontos + 3).trimmed();
        } else if (posTraco >= 0) {
            hab.nome = limparNegrito(t.left(posTraco));
            hab.descricao = t.mid(posTraco + 3).trimmed();
        } else {
            hab.nome = limparNegrito(t);
        }
        hab.categoria = marcador;
        habilidades.append(hab);
        fim++;
    }

    for (int i = fim - 1; i >= inicio; --i)
        restoLinhas.removeAt(i);

    return habilidades;
}

// Procura uma seção "Inventário:" (com um peso total opcional na mesma linha,
// ex. "Inventário: Peso 8.5" — ou em negrito, "*Inventario:*") e extrai os
// itens até encontrar "Habilidades:", uma linha separadora (====) ou o fim do
// texto. Aceita grupos separados por linha em branco, quantidade no início
// ("2 kit médicos"), "PESO x" em qualquer parte da linha, "R$ x" (vai pro
// dinheiro da ficha em vez de virar item), e itens "container" — que terminam
// em ":" ou vêm em **negrito** (ex. "bloco de notas:", "**Metal Blades**") —
// cujas linhas seguintes começando com "-" ou "N-"/"N)" (bullet numerado)
// viram a descrição do item, ao invés de itens novos. Uma linha "-algo" que
// não segue um container vira ela mesma um item novo (ex. "-1 amolador").
// Remove as linhas consumidas de "linhas" e devolve o peso total (se achado)
// em "notaPesoTotal" e a soma de "R$" encontrada em "dinheiroSaida".
QVector<ItemInventario> extrairInventario(QStringList &linhas, QString &notaPesoTotal, double *dinheiroSaida = nullptr)
{
    QVector<ItemInventario> itens;

    static const QRegularExpression rePeso("\\bpeso\\b\\s*:?\\s*([\\d.,]+)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reQuantidade("^(\\d+)\\s*[xX]?\\s+(.+)$");
    static const QRegularExpression reDinheiro("^r\\$\\s*:?\\s*([\\d.,]+)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reBulletNumerado("^\\d+\\s*[-.)]\\s*");

    int inicio = -1;
    QString restoHeader;
    for (int i = 0; i < linhas.size(); ++i) {
        if (normalizarMarcador(linhas[i]) == "inventario" || normalizarMarcador(linhas[i]) == "inventário") {
            inicio = i;
            restoHeader.clear();
            break;
        }
        const QString t = linhas[i].trimmed();
        if (t.toLower().startsWith("invent") && t.contains(':')) {
            inicio = i;
            restoHeader = t.section(':', 1).trimmed();
            break;
        }
    }
    if (inicio < 0)
        return itens;

    int containerIdx = -1;
    int fim = inicio + 1;
    while (fim < linhas.size()) {
        const QString bruta = linhas[fim].trimmed();

        if (linhaDivisoria(bruta) || normalizarMarcador(bruta) == "habilidades")
            break;

        if (bruta.isEmpty()) {
            containerIdx = -1;
            fim++;
            continue;
        }

        const bool ehNegrito = bruta.startsWith("**") && bruta.endsWith("**") && bruta.length() > 4;
        const QString t = ehNegrito ? limparNegrito(bruta) : bruta;

        const QRegularExpressionMatch mDinheiro = reDinheiro.match(t);
        if (mDinheiro.hasMatch()) {
            if (dinheiroSaida)
                *dinheiroSaida += mDinheiro.captured(1).simplified().replace(',', '.').toDouble();
            containerIdx = -1;
            fim++;
            continue;
        }

        const bool ehBulletTraco = t.startsWith('-');
        const QRegularExpressionMatch mBulletNum = reBulletNumerado.match(t);
        const bool ehBulletNumerado = mBulletNum.hasMatch();
        const bool ehBullet = ehBulletTraco || ehBulletNumerado;

        if (ehBullet && containerIdx >= 0) {
            ItemInventario &item = itens[containerIdx];
            const QString conteudo = ehBulletTraco ? t.mid(1).trimmed() : t.mid(mBulletNum.capturedLength(0)).trimmed();
            if (!item.utilidade.isEmpty())
                item.utilidade += '\n';
            item.utilidade += conteudo;
            fim++;
            continue;
        }

        QString semPeso = ehBulletTraco ? t.mid(1).trimmed() : t;
        QString notaPeso;
        const QRegularExpressionMatch mPeso = rePeso.match(semPeso);
        if (mPeso.hasMatch()) {
            notaPeso = QString("Peso %1").arg(mPeso.captured(1));
            semPeso.remove(mPeso.capturedStart(0), mPeso.capturedLength(0));
            semPeso = semPeso.simplified();
        }

        const bool ehContainer = ehNegrito || semPeso.endsWith(':');
        if (semPeso.endsWith(':'))
            semPeso.chop(1);
        semPeso = semPeso.simplified();

        int quantidade = 1;
        const QRegularExpressionMatch mQtd = reQuantidade.match(semPeso);
        if (mQtd.hasMatch()) {
            quantidade = mQtd.captured(1).toInt();
            semPeso = mQtd.captured(2).simplified();
        }

        ItemInventario item;
        item.nome = semPeso;
        item.quantidade = quantidade;
        item.utilidade = notaPeso;
        itens.append(item);

        containerIdx = ehContainer ? itens.size() - 1 : -1;
        fim++;
    }

    const QRegularExpressionMatch mPesoTotal = rePeso.match(restoHeader);
    if (mPesoTotal.hasMatch())
        notaPesoTotal = QString("Peso total do inventário: %1").arg(mPesoTotal.captured(1));

    for (int i = fim - 1; i >= inicio; --i)
        linhas.removeAt(i);

    return itens;
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

    QStringList linhas = texto.split('\n');
    QString notaPesoTotal;
    double dinheiro = 0.0;
    ficha.inventario = extrairInventario(linhas, notaPesoTotal, &dinheiro);
    ficha.dinheiro = dinheiro;
    if (!notaPesoTotal.isEmpty())
        notas << notaPesoTotal;

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

CharacterSheet CharacterSheet::importarDeTexto(const QString &textoOriginal, bool *usouFormatoGenerico)
{
    const QString texto = corrigirMojibake(textoOriginal);

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
    static const QRegularExpression reVidaMax("^vida\\s*m[aá]x\\w*\\s*:\\s*(\\d+)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reSanidade("^sanidade\\s*:\\s*(\\d+)\\s*/\\s*(\\d+)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reSanidadeMax("^sanidade\\s*m[aá]x\\w*\\s*:\\s*(\\d+)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reIdade("^idade\\s*:\\s*(\\d+)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reAltura("^altura\\s*:\\s*(.*)$", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reDiscernimento("^discernimento\\s*:\\s*(\\d+)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reExperienciaParanormal(
        "^experi[eê]ncia\\s+paranormal\\s*:\\s*(\\d+)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reInt("(-?\\d+)");

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
            m = reVidaMax.match(t);
            if (m.hasMatch()) {
                vidaMax = m.captured(1).toInt();
                vidaAtual = vidaMax;
                continue;
            }
            m = reSanidade.match(t);
            if (m.hasMatch()) {
                sanidadeAtual = m.captured(1).toInt();
                sanidadeMax = m.captured(2).toInt();
                continue;
            }
            m = reSanidadeMax.match(t);
            if (m.hasMatch()) {
                sanidadeMax = m.captured(1).toInt();
                sanidadeAtual = sanidadeMax;
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
            m = reExperienciaParanormal.match(t);
            if (m.hasMatch()) {
                discernimento = m.captured(1).toInt();
                continue;
            }
            if (normalizarMarcador(t) == "atributos" || (t.toLower().contains("atributos") && t.contains('=') && !t.toLower().contains("sub"))) {
                estado = Atributos;
                marcadorAtributosEncontrado = true;
                continue;
            }
            if (linhaDivisoria(t)) // separador solto (ex. "=========") antes do marcador — não vai pra notas
                continue;
            headerLinhas << linhaOriginal;
            continue;
        }

        if (estado == Atributos) {
            const QString marcadorNormalizado = normalizarMarcador(t);
            if (marcadorNormalizado.contains("especializa")
                || (marcadorNormalizado.contains("sub") && marcadorNormalizado.contains("atributo"))) {
                estado = SubAtributos;
                continue;
            }
            if (t.isEmpty() || !t.contains(':') || linhaDivisoria(t))
                continue;

            const QString valorTexto = t.section(':', 1).trimmed();
            const QRegularExpressionMatch mValor = reInt.match(valorTexto);

            Atributo a;
            a.nome = t.section(':', 0, 0).trimmed();
            a.valor = mValor.hasMatch() ? mValor.captured(1).toInt() : 0;
            atributos.append(a);
            continue;
        }

        if (estado == SubAtributos) {
            if (linhaSeparadora(t) || linhaDivisoria(t)) {
                estado = Resto;
                continue;
            }
            if (t.isEmpty())
                continue;

            // cabeçalho de grupo estilo "-Nome-" (sem ':')
            const bool grupoEstiloTraco = t.startsWith('-') && t.endsWith('-') && t.length() > 1 && !t.contains(':');
            // cabeçalho de grupo estilo "Nome=" (um "=" só no fim, sem ':')
            const bool grupoEstiloIgual = !grupoEstiloTraco && t.endsWith('=') && t.count('=') == 1 && !t.contains(':');

            if (grupoEstiloTraco || grupoEstiloIgual) {
                const QString nomeGrupo = grupoEstiloTraco ? t.mid(1, t.length() - 2).trimmed() : t.chopped(1).trimmed();
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

            // linha de sub-atributo: "-Nome: valor" (estilo antigo) ou "Nome: valor"/"Nome:-" (estilo Ordem)
            if (t.contains(':')) {
                const QString semTraco = t.startsWith('-') ? t.mid(1) : t;
                const QString valorTexto = semTraco.section(':', 1).trimmed();
                const QRegularExpressionMatch mValor = reInt.match(valorTexto);

                SubAtributo sub;
                sub.nome = semTraco.section(':', 0, 0).trimmed();
                sub.valor = mValor.hasMatch() ? mValor.captured(1).toInt() : 0;
                if (atributoAtual && !sub.nome.isEmpty())
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

    QVector<Habilidade> habilidades = extrairHabilidades(restoLinhas, "Habilidades");
    habilidades += extrairHabilidades(restoLinhas, "Rituais");
    QString notaPesoTotal;
    double dinheiro = 0.0;
    const QVector<ItemInventario> inventario = extrairInventario(restoLinhas, notaPesoTotal, &dinheiro);

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
    ficha.inventario = inventario;
    ficha.dinheiro = dinheiro;

    QStringList partesDescricao;
    const QString headerTexto = headerLinhas.join('\n').trimmed();
    const QString restoTexto = restoLinhas.join('\n').trimmed();
    if (!headerTexto.isEmpty())
        partesDescricao << headerTexto;
    if (!notaPesoTotal.isEmpty())
        partesDescricao << notaPesoTotal;
    if (!restoTexto.isEmpty())
        partesDescricao << restoTexto;
    ficha.descricao = partesDescricao.join("\n\n");

    return ficha;
}
