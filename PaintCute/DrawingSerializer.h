#pragma once

#include <QString>
#include "Drawing.h"

class DrawingSerializer {
public:
    enum class Result { Ok, FileOpenError, FormatError };

    static Result save(const QString& filename, const Drawing& drawing);
    static Result load(const QString& filename, Drawing& drawing);
};