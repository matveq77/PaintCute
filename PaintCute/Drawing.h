#pragma once

#include <QVector>
#include <QDataStream>
#include "Shape.h"
#include "Connection.h"
#include <memory>

class Drawing {
public:
    Drawing() = default;
    ~Drawing();

    Drawing(const Drawing&) = delete;
    Drawing& operator=(const Drawing&) = delete;
    Drawing(Drawing&&) = default;
    Drawing& operator=(Drawing&&) = default;

    void clear();

    QVector<std::unique_ptr<Shape>>& shapes() { return shapes_; }
    const QVector<std::unique_ptr<Shape>>& shapes() const { return shapes_; }

    QVector<Connection>& connections() { return connections_; }
    const QVector<Connection>& connections() const { return connections_; }

    bool serialize(QDataStream& out) const;
    bool deserialize(QDataStream& in);

private:
    QVector<std::unique_ptr<Shape>> shapes_;
    QVector<Connection> connections_;
};