#include "DrawingSerializer.h"
#include <QFile>
#include <QDataStream>

DrawingSerializer::Result DrawingSerializer::save(const QString& filename, const Drawing& drawing)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return Result::FileOpenError;

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_0);

    return drawing.serialize(out) ? Result::Ok : Result::FormatError;
}

DrawingSerializer::Result DrawingSerializer::load(const QString& filename, Drawing& drawing)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return Result::FileOpenError;

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);

    return drawing.deserialize(in) ? Result::Ok : Result::FormatError;
}