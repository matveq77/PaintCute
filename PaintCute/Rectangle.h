#pragma once

#include "Shape.h"

class Rectangle : public Shape {
public:
    Rectangle();
    explicit Rectangle(const QPoint& start, const QPoint& end);

    void draw(QPainter& painter) const override;
    bool isPointInShape(const QPoint& p) const override;
    void move(const QPoint& delta) override;

    void serialize(QDataStream& out) const override;
    void deserialize(QDataStream& in) override;

    QPoint getCenter() const override;
    QRect getBoundingRect() const override;

private:
    QRect getRect() const;
};