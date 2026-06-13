#pragma once

#include <QPoint>
#include <QDataStream>

class QPainter;

class Connection {
public:
    Connection();
    Connection(int fromShapeId, int toShapeId);

    void draw(QPainter& painter, const QPoint& fromCenter, const QPoint& toCenter) const;

    int getFromShapeId() const { return fromShapeId_; }
    int getToShapeId() const { return toShapeId_; }

    void setFromShapeId(int id) { fromShapeId_ = id; }
    void setToShapeId(int id) { toShapeId_ = id; }

    void serialize(QDataStream& out) const;
    void deserialize(QDataStream& in);

private:
    int fromShapeId_ = -1;
    int toShapeId_ = -1;

    static constexpr int ARROW_SIZE = 10;
};
