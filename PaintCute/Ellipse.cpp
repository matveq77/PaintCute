#include "Ellipse.h"
#include <QPainter>
#include <cmath>

Ellipse::Ellipse() : Shape(Type::Ellipse) {}

Ellipse::Ellipse(const QPoint& start, const QPoint& end)
    : Shape(Type::Ellipse) {
    start_ = start;
    end_ = end;
}

void Ellipse::draw(QPainter& painter) const {
    QRect rect = getNormalizedRect();
    painter.drawEllipse(rect);
}

bool Ellipse::isPointInShape(const QPoint& p) const {
    QRect rect = getNormalizedRect();
    if (rect.width() <= 0 || rect.height() <= 0) return false;

    QPointF center = rect.center();
    double rx = rect.width() / 2.0;
    double ry = rect.height() / 2.0;
    double dx = p.x() - center.x();
    double dy = p.y() - center.y();

    return (dx * dx) / (rx * rx) + (dy * dy) / (ry * ry) <= 1.0;
}

QRect Ellipse::getBoundingRect() const {
    return getNormalizedRect();
}