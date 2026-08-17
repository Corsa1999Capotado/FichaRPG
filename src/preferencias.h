#pragma once

// Preferências gerais do app (não pertencem a nenhuma ficha/pasta específica),
// persistidas num JSON pequeno na pasta de dados do usuário.
namespace Preferencias
{
int colunasGrid(); // 2, 3 ou 4 (padrão 3)
void definirColunasGrid(int colunas);

bool confirmarAntesDeExcluir(); // padrão true
void definirConfirmarAntesDeExcluir(bool confirmar);
}
