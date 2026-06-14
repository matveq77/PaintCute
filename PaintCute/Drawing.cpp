#include "Drawing.h"

Drawing::~Drawing() = default;

void Drawing::clear() {
    shapes_.clear();
    connections_.clear();
}

bool Drawing::serialize(QDataStream& out) const {
    out << static_cast<quint32>(shapes_.size());
    for (const auto& shape : shapes_) {
        out << static_cast<int>(shape->getType());
        shape->serialize(out);
    }

    out << static_cast<quint32>(connections_.size());
    for (const auto& conn : connections_) {
        conn.serialize(out);
    }

    return out.status() == QDataStream::Ok;
}

bool Drawing::deserialize(QDataStream& in) {
    QVector<std::unique_ptr<Shape>> loadedShapes;
    QVector<Connection> loadedConnections;

    quint32 shapeCount;
    in >> shapeCount;
    loadedShapes.reserve(shapeCount);

    for (quint32 i = 0; i < shapeCount; ++i) {
        int typeInt;
        in >> typeInt;
        auto shape = Shape::createFromType(static_cast<Shape::Type>(typeInt));
        if (!shape) {
            return false;
        }
        shape->deserialize(in);
        loadedShapes.append(std::move(shape));
    }

    quint32 connCount;
    in >> connCount;
    loadedConnections.reserve(connCount);
    for (quint32 i = 0; i < connCount; ++i) {
        Connection conn;
        conn.deserialize(in);
        loadedConnections.append(conn);
    }

    if (in.status() != QDataStream::Ok) {
        return false;
    }

    clear();
    shapes_ = std::move(loadedShapes);
    connections_ = std::move(loadedConnections);
    return true;
}