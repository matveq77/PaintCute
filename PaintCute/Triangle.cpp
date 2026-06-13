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
    QRect rect = getRect();

    QPoint top(rect.center().x(), rect.top());
    QPoint bottomLeft(rect.left(), rect.bottom());
    QPoint bottomRight(rect.right(), rect.bottom());

    QPolygon triangle;
    triangle << top << bottomLeft << bottomRight;

    painter.drawPolygon(triangle);
}

bool Triangle::isPointInShape(const QPoint& p) const {
    QRect rect = getRect();

    // Create triangle inscribed in rectangle
    QPoint top(rect.center().x(), rect.top());
    QPoint bottomLeft(rect.left(), rect.bottom());
    QPoint bottomRight(rect.right(), rect.bottom());

    QPolygon triangle;
    triangle << top << bottomLeft << bottomRight;

    return triangle.containsPoint(p, Qt::OddEvenFill);
}

void Triangle::move(const QPoint& delta) {
    start_ += delta;
    end_ += delta;
}

void Triangle::serialize(QDataStream& out) const {
    out << start_ << end_;
}

void Triangle::deserialize(QDataStream& in) {
    in >> start_ >> end_;
}

QPoint Triangle::getCenter() const {
    return getRect().center();
}

QRect Triangle::getBoundingRect() const {
    return getRect();
}

QRect Triangle::getRect() const {
    return QRect(start_, end_).normalized();
}
