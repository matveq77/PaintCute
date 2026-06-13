#pragma once

#include <QPoint>
#include <QRect>
#include <QDataStream>
#include <memory>

class QPainter;

class Shape {
public:
    enum class Type {
        Rectangle = 0,
        Ellipse = 1,
        Triangle = 2
    };

    explicit Shape(Type type);
    virtual ~Shape() = default;

    virtual void draw(QPainter& painter) const = 0;
    virtual bool isPointInShape(const QPoint& p) const = 0;
    virtual void move(const QPoint& delta) = 0;

    virtual void serialize(QDataStream& out) const = 0;
    virtual void deserialize(QDataStream& in) = 0;

    virtual QPoint getCenter() const = 0;
    virtual QRect getBoundingRect() const = 0;

    Type getType() const { return type_; }
    const QPoint& getStart() const { return start_; }
    const QPoint& getEnd() const { return end_; }

    void setStart(const QPoint& p) { start_ = p; }
    void setEnd(const QPoint& p) { end_ = p; }

    static std::unique_ptr<Shape> createFromType(Type type);

protected:
    Type type_;
    QPoint start_;
    QPoint end_;
};

