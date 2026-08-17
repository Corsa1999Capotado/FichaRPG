#pragma once

#include <QPixmap>
#include <QPointF>
#include <QSize>

// Escala e recorta uma imagem pro tamanho exato de um widget, mantendo
// visível o ponto "foco" (coordenadas normalizadas 0..1 dentro da imagem
// original) o mais centralizado possível — sem deixar sobrar borda vazia.
// foco = (0.5, 0.5) reproduz o comportamento antigo (sempre centralizado).
namespace ImagemUtil
{
QPixmap recortarComFoco(const QPixmap &origem, const QSize &tamanhoAlvo, QPointF foco);
}
