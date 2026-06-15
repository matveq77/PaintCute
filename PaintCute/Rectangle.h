#pragma once

#include "Shape.h"

class Rectangle : public Shape {
public:
    Rectangle();
    explicit Rectangle(const QPoint& start, const QPoint& end);

    void draw(QPainter& painter) const override;
    bool isPointInShape(const QPoint& p) const override;
    QRect getBoundingRect() const override;
private:
    
};