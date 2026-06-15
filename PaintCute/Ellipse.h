#pragma once

#include "Shape.h"
#include <QRect>

class Ellipse : public Shape
{
public:
    Ellipse();
    Ellipse(const QPoint& start, const QPoint& end);

    void draw(QPainter& painter) const override;
    bool isPointInShape(const QPoint& p) const override;
    QRect getBoundingRect() const override;
private:
    
};