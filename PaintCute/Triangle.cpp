#include "Triangle.h"
#include <QPainter>
#include <QRect>

Triangle::Triangle() : Shape(Type::Triangle) {}

Triangle::Triangle(const QPoint& start, const QPoint& end)
    : Shape(Type::Triangle) {
    start_ = start;
    end_ = end;
}

void Triangle::draw(QPainter& painter) const {
    QRect rect = getNormalizedRect();

    QPoint top(rect.center().x(), rect.top());
    QPoint bottomLeft(rect.left(), rect.bottom());
    QPoint bottomRight(rect.right(), rect.bottom());

    QPolygon triangle;
    triangle << top << bottomLeft << bottomRight;

    painter.drawPolygon(triangle);
}

bool Triangle::isPointInShape(const QPoint& p) const {
    QRect rect = getNormalizedRect();

    QPoint top(rect.center().x(), rect.top());
    QPoint bottomLeft(rect.left(), rect.bottom());
    QPoint bottomRight(rect.right(), rect.bottom());

    QPolygon triangle;
    triangle << top << bottomLeft << bottomRight;

    return triangle.containsPoint(p, Qt::OddEvenFill);
}

QRect Triangle::getBoundingRect() const {
    return getNormalizedRect();
}
