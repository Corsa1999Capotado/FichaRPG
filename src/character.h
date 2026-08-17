#pragma once

#include <QString>
#include <QVector>
#include <QJsonObject>

struct SubAtributo
{
    QString nome;
    int valor = 0;
};

struct Atributo
{
    QString nome;
    int valor = 0;
    bool automatico = false; // se true, valor é recalculado a partir de "formula"
    QString formula;         // ex: "Fortitude * 2 + 10"
    QString descricao;       // texto livre opcional (o que o atributo significa/faz)
    QVector<SubAtributo> subAtributos;
};

// Stat extra definido pelo mestre além de vida/sanidade/discernimento (ex:
// "Estresse", "Munição"). max == 0 = sem máximo, só um contador livre.
struct RecursoCustom
{
    QString nome;
    int atual = 0;
    int max = 0;
};

struct ItemInventario
{
    QString nome;
    int quantidade = 1;
    QString utilidade;
    bool contavel = true; // false = item único (ex: uma espada específica) — sem quantidade/controles de +/-
};

struct Habilidade
{
    QString nome;
    QString descricao;
    QString categoria; // nome da seção (ex: "Rituais", "Habilidades de combate"); vazio = sem seção
};

class CharacterSheet
{
public:
    QString nome;
    QString imagemArquivo; // nome do arquivo dentro da pasta de imagens (sem caminho completo)
    // Ponto da imagem original (0,0 = canto superior esquerdo; 1,1 = canto
    // inferior direito) que fica centralizado no recorte da miniatura/foto —
    // permite escolher qual parte de uma imagem grande/vertical aparece, em
    // vez de sempre centralizar automaticamente. Padrão (0.5, 0.5) = centro,
    // igual ao comportamento antigo.
    double imagemFocoX = 0.5;
    double imagemFocoY = 0.5;

    int idade = 0;
    QString altura;

    int vidaAtual = 0;
    int vidaMax = 0;
    bool vidaAutomatica = false; // se true, vidaMax é recalculado a partir de "formulaVida"
    QString formulaVida;

    int sanidadeAtual = 0;
    int sanidadeMax = 0;
    bool sanidadeAutomatica = false; // se true, sanidadeMax é recalculado a partir de "formulaSanidade"
    QString formulaSanidade;

    int discernimento = 0; // 0-100 (%)
    bool discernimentoAutomatico = false;
    QString formulaDiscernimento;

    QVector<Atributo> atributos;
    double dinheiro = 0.0;
    QVector<ItemInventario> inventario;
    QVector<Habilidade> habilidades;
    QVector<RecursoCustom> recursos;

    QString descricao; // notas/background livres (rich text, com imagens embutidas no meio)

    QJsonObject toJson() const;
    static CharacterSheet fromJson(const QJsonObject &obj);

    bool salvarEmArquivo(const QString &caminho) const;
    static bool carregarDeArquivo(const QString &caminho, CharacterSheet &destino);

    // Ficha em branco com os 6 atributos e sub-atributos do sistema ARCA
    static CharacterSheet modeloArca();

    // Interpreta um .txt no formato de ficha "solta" (nome, idade, altura, vida,
    // discernimento, atributos, sub-atributos, habilidades, inventário e o resto
    // cai nas notas). A seção de inventário é reconhecida por uma linha
    // "Inventário:" (com peso total opcional, ex. "Inventário: Peso 8.5”),
    // seguida pelos itens — um por linha, com quantidade opcional no início
    // ("2 kit médicos"), "PESO x"/"R$ x" em qualquer parte da linha (R$ vai pro
    // dinheiro da ficha), e itens que terminam em ":" ou vêm em **negrito**
    // viram um item cujas linhas seguintes começando com "-" formam a descrição.
    // Também aceita o modelo "Ordem" (marcadores em *negrito* tipo "*Atributos:*"
    // em vez de "====Atributos====", grupos de sub-atributo "Nome=" além do
    // "-Nome-" tradicional, valores com texto sujo atrás como "15 (+3) [...", e
    // corrige sozinho mojibake comum (arquivo UTF-8 lido como Latin-1).
    // Se o texto não seguir nenhum marcador conhecido, cai num modo genérico que
    // tenta detectar pares "Nome: número" como atributos — nesse caso,
    // *usouFormatoGenerico (se não for nullptr) vira true. O inventário é
    // extraído em ambos os modos.
    static CharacterSheet importarDeTexto(const QString &texto, bool *usouFormatoGenerico = nullptr);
};
