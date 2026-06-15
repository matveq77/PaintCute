#pragma once

#include "Shape.h"

class Triangle : public Shape {
public:
    Triangle();
    explicit Triangle(const QPoint& start, const QPoint& end);

    void draw(QPainter& painter) const override;
    bool isPointInShape(const QPoint& p) const override;
    QRect getBoundingRect() const override;
private:
    
};
