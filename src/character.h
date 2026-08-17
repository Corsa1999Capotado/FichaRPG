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
    QVector<SubAtributo> subAtributos;
};

struct ItemInventario
{
    QString nome;
    int quantidade = 1;
    QString utilidade;
};

struct Habilidade
{
    QString nome;
    QString descricao;
};

class CharacterSheet
{
public:
    QString nome;
    QString imagemArquivo; // nome do arquivo dentro da pasta de imagens (sem caminho completo)

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

    QString descricao; // notas/background livres (rich text, com imagens embutidas no meio)

    QJsonObject toJson() const;
    static CharacterSheet fromJson(const QJsonObject &obj);

    bool salvarEmArquivo(const QString &caminho) const;
    static bool carregarDeArquivo(const QString &caminho, CharacterSheet &destino);

    // Ficha em branco com os 6 atributos e sub-atributos do sistema ARCA
    static CharacterSheet modeloArca();

    // Interpreta um .txt no formato de ficha "solta" (nome, idade, altura, vida,
    // discernimento, atributos, sub-atributos, habilidades e o resto cai nas notas).
    // Se o texto não seguir os marcadores conhecidos (====Atributos==== etc.), cai
    // num modo genérico que tenta detectar pares "Nome: número" como atributos —
    // nesse caso, *usouFormatoGenerico (se não for nullptr) vira true.
    static CharacterSheet importarDeTexto(const QString &texto, bool *usouFormatoGenerico = nullptr);
};
