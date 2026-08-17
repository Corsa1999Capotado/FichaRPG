#pragma once

#include <QMap>
#include <QString>

// Avaliador simples de expressões aritméticas (+, -, *, /, parênteses, números
// e variáveis) usado pelas fórmulas de cálculo automático das fichas.
namespace FormulaEngine
{
// *ok fica false se a fórmula for inválida ou usar uma variável desconhecida
double avaliar(const QString &formula, const QMap<QString, double> &variaveis, bool *ok);
}
