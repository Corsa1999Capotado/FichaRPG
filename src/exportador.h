#pragma once

#include <QString>

#include "character.h"

class QWidget;

// Exportação da ficha em diferentes formatos.
namespace Exportador
{
bool exportarJson(const CharacterSheet &ficha, const QString &caminho);
bool exportarImagem(QWidget *origem, const QString &caminho); // screenshot (grab) do widget, salvo como PNG
bool exportarPdf(const CharacterSheet &ficha, const QString &caminho); // documento formatado pra impressão
}
