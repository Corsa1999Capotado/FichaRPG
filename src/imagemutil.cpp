#include "imagemutil.h"

#include <QPoint>
#include <QRect>

#include <algorithm>
#include <cmath>

namespace ImagemUtil
{
QPixmap recortarComFoco(const QPixmap &origem, const QSize &tamanhoAlvo, QPointF foco)
{
    if (origem.isNull() || tamanhoAlvo.isEmpty())
        return QPixmap();

    const qreal escala =
        std::max(qreal(tamanhoAlvo.width()) / origem.width(), qreal(tamanhoAlvo.height()) / origem.height());
    const QSize tamanhoEscalado(std::max(1, int(std::ceil(origem.width() * escala))),
                                 std::max(1, int(std::ceil(origem.height() * escala))));
    const QPixmap escalada = origem.scaled(tamanhoEscalado, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    const qreal fx = std::clamp(foco.x(), 0.0, 1.0);
    const qreal fy = std::clamp(foco.y(), 0.0, 1.0);

    qreal x = fx * escalada.width() - tamanhoAlvo.width() / 2.0;
    qreal y = fy * escalada.height() - tamanhoAlvo.height() / 2.0;
    x = std::clamp(x, 0.0, qreal(escalada.width() - tamanhoAlvo.width()));
    y = std::clamp(y, 0.0, qreal(escalada.height() - tamanhoAlvo.height()));

    return escalada.copy(QRect(QPoint(qRound(x), qRound(y)), tamanhoAlvo));
}
}
