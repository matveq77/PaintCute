#pragma once

#include <QVector>
#include <QDataStream>
#include "Shape.h"
#include "Connection.h"

class Drawing {
public:
    Drawing() = default;
    ~Drawing();

    Drawing(const Drawing&) = delete;
    Drawing& operator=(const Drawing&) = delete;
    Drawing(Drawing&&) = default;
    Drawing& operator=(Drawing&&) = default;

    void clear();

    QVector<Shape*>& shapes() { return shapes_; }
    const QVector<Shape*>& shapes() const { return shapes_; }

    QVector<Connection>& connections() { return connections_; }
    const QVector<Connection>& connections() const { return connections_; }

    bool serialize(QDataStream& out) const;
    bool deserialize(QDataStream& in);

private:
    QVector<Shape*> shapes_;
    QVector<Connection> connections_;
};