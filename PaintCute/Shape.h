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
  
    void move(const QPoint& delta);

    void serialize(QDataStream& out) const;
    void deserialize(QDataStream& in);

    QPoint getCenter() const;
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
    
    QRect getNormalizedRect() const {
        return QRect(start_, end_).normalized();
    }
};

