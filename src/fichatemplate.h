#pragma once

#include <QString>
#include <QJsonObject>

#include "character.h"

// Um template é uma ficha "base" (mesma estrutura de CharacterSheet) com um
// nome de exibição próprio, salva separadamente em /templates/ e reutilizável
// pra criar novas fichas.
struct FichaTemplate
{
    QString nomeTemplate;
    CharacterSheet base;

    QJsonObject toJson() const;
    static FichaTemplate fromJson(const QJsonObject &obj);

    bool salvarEmArquivo(const QString &caminho) const;
    static bool carregarDeArquivo(const QString &caminho, FichaTemplate &destino);
};
