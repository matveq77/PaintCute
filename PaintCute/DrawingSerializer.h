#pragma once
#include <QVector>
#include <QString>
#include <Shape.h>
#include <Connection.h>

class DrawingSerializer {
public:
    enum class Result { Ok, FileOpenError, FormatError };

    static Result save(const QString& filename,
        const QVector<Shape*>& shapes,
        const QVector<Connection>& connections);

    static Result load(const QString& filename,
        QVector<Shape*>& shapes,
        QVector<Connection>& connections);
};

