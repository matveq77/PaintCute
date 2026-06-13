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
    QRect rect = getRect();
    painter.drawRect(rect);
}

bool Rectangle::isPointInShape(const QPoint& p) const {
    return getRect().contains(p);
}

void Rectangle::move(const QPoint& delta) {
    start_ += delta;
    end_ += delta;
}

void Rectangle::serialize(QDataStream& out) const {
    out << start_ << end_;
}

void Rectangle::deserialize(QDataStream& in) {
    in >> start_ >> end_;
}

QPoint Rectangle::getCenter() const {
    return getRect().center();
}

QRect Rectangle::getBoundingRect() const {
    return getRect();
}

QRect Rectangle::getRect() const {
    return QRect(start_, end_).normalized();
}