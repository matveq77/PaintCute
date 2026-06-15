#include "Rectangle.h"
#include <QPainter>
#include <QRect>

Rectangle::Rectangle() : Shape(Type::Rectangle) {}

Rectangle::Rectangle(const QPoint& start, const QPoint& end)
    : Shape(Type::Rectangle) {
    start_ = start;
    end_ = end;
}

void Rectangle::draw(QPainter& painter) const {
    QRect rect = getNormalizedRect();
    painter.drawRect(rect);
}

bool Rectangle::isPointInShape(const QPoint& p) const {
    return getNormalizedRect().contains(p);
}
QRect Rectangle::getBoundingRect() const {
    return getNormalizedRect();
}