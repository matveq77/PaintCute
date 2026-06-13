#include "DrawingSerializer.h"
#include "QFile.h"
//#include "QDataStream.h"

DrawingSerializer::Result DrawingSerializer::save(
    const QString& filename,
    const QVector<Shape*>& shapes,
    const QVector<Connection>& connections)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return Result::FileOpenError;

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_0);

    out << static_cast<quint32>(shapes.size());
    for (const auto& shape : shapes) {
        out << static_cast<int>(shape->getType());
        shape->serialize(out);
    }

    out << static_cast<quint32>(connections.size());
    for (const auto& conn : connections) {
        conn.serialize(out);
    }

    return (out.status() == QDataStream::Ok) ? Result::Ok : Result::FormatError;
}

DrawingSerializer::Result DrawingSerializer::load(
    const QString& filename,
    QVector<Shape*>& shapes,
    QVector<Connection>& connections)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return Result::FileOpenError;

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);

    quint32 shapeCount;
    in >> shapeCount;
    QVector<Shape*> loadedShapes;
    loadedShapes.reserve(shapeCount);

    for (quint32 i = 0; i < shapeCount; ++i) {
        int typeInt;
        in >> typeInt;
        auto shape = Shape::createFromType(static_cast<Shape::Type>(typeInt));
        if (!shape) {
            for (auto* s : loadedShapes) delete s;
            return Result::FormatError;
        }
        shape->deserialize(in);
        loadedShapes.append(shape.release());
    }

    quint32 connCount;
    in >> connCount;
    QVector<Connection> loadedConnections;
    loadedConnections.reserve(connCount);
    for (quint32 i = 0; i < connCount; ++i) {
        Connection conn;
        conn.deserialize(in);
        loadedConnections.append(conn);
    }

    if (in.status() != QDataStream::Ok) {
        for (auto* s : loadedShapes) delete s;
        return Result::FormatError;
    }

    shapes = loadedShapes;
    connections = loadedConnections;
    return Result::Ok;
}