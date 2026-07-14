#include "DatabaseManager.h"
#include "Drawing.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QDataStream>
#include <QBuffer>

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent) {
}

DatabaseManager::~DatabaseManager() {
    if (db_.isValid() && db_.isOpen()) {
        db_.close();
    }
}

bool DatabaseManager::open(const QString& host, int port, const QString& dbName,
                            const QString& user, const QString& password,
                            QString& error) {
    const QString connectionName = "PaintCuteConnection";
    if (db_.isValid() && db_.isOpen()) {
        db_.close();
    }
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }

    db_ = QSqlDatabase::addDatabase("QPSQL", connectionName);
    db_.setHostName(host);
    db_.setPort(port);
    db_.setDatabaseName(dbName);
    db_.setUserName(user);
    db_.setPassword(password);

    if (!db_.open()) {
        error = db_.lastError().text();
        return false;
    }

    return ensureSchema(error);
}

bool DatabaseManager::isOpen() const {
    return db_.isValid() && db_.isOpen();
}

bool DatabaseManager::ensureSchema(QString& error) {
    if (!db_.isOpen()) {
        error = "Database is not open.";
        return false;
    }

    QSqlQuery query(db_);
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS drawings (
            id SERIAL PRIMARY KEY,
            name TEXT NOT NULL,
            created_at TIMESTAMP WITH TIME ZONE DEFAULT now()
        )
    )")) {
        error = query.lastError().text();
        return false;
    }

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS shapes (
            id SERIAL PRIMARY KEY,
            drawing_id INTEGER NOT NULL REFERENCES drawings(id) ON DELETE CASCADE,
            shape_order INTEGER NOT NULL,
            type INTEGER NOT NULL,
            start_x INTEGER NOT NULL,
            start_y INTEGER NOT NULL,
            end_x INTEGER NOT NULL,
            end_y INTEGER NOT NULL
        )
    )")) {
        error = query.lastError().text();
        return false;
    }

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS connections (
            id SERIAL PRIMARY KEY,
            drawing_id INTEGER NOT NULL REFERENCES drawings(id) ON DELETE CASCADE,
            from_shape_id INTEGER NOT NULL REFERENCES shapes(id) ON DELETE CASCADE,
            to_shape_id INTEGER NOT NULL REFERENCES shapes(id) ON DELETE CASCADE
        )
    )")) {
        error = query.lastError().text();
        return false;
    }

    return true;
}

QList<QPair<int, QString>> DatabaseManager::listDrawings(QString& error) {
    if (!db_.isOpen()) {
        error = "Database connection is not open.";
        return {};
    }

    QList<QPair<int, QString>> results;
    QSqlQuery query(db_);
    if (!query.exec("SELECT id, name FROM drawings ORDER BY created_at DESC")) {
        error = query.lastError().text();
        return {};
    }

    while (query.next()) {
        results.append({ query.value(0).toInt(), query.value(1).toString() });
    }

    return results;
}

bool DatabaseManager::saveDrawing(const QString& name, const Drawing& drawing, QString& error) {
    if (!db_.isOpen()) {
        error = "Database connection is not open.";
        return false;
    }

    if (!db_.transaction()) {
        error = db_.lastError().text();
        return false;
    }

    QSqlQuery query(db_);
    query.prepare("INSERT INTO drawings (name) VALUES (:name) RETURNING id");
    query.bindValue(":name", name);
    if (!query.exec() || !query.next()) {
        error = query.lastError().text();
        db_.rollback();
        return false;
    }

    int drawingId = query.value(0).toInt();
    QHash<int, int> shapeIdMap;

    for (int index = 0; index < drawing.shapeCount(); ++index) {
        Shape* shape = drawing.shapeAt(index);
        if (!shape) continue;

        query.prepare(R"(
            INSERT INTO shapes (drawing_id, shape_order, type, start_x, start_y, end_x, end_y)
            VALUES (:drawing_id, :shape_order, :type, :start_x, :start_y, :end_x, :end_y)
            RETURNING id
        )");
        query.bindValue(":drawing_id", drawingId);
        query.bindValue(":shape_order", index);
        query.bindValue(":type", static_cast<int>(shape->getType()));
        query.bindValue(":start_x", shape->getStart().x());
        query.bindValue(":start_y", shape->getStart().y());
        query.bindValue(":end_x", shape->getEnd().x());
        query.bindValue(":end_y", shape->getEnd().y());

        if (!query.exec() || !query.next()) {
            error = query.lastError().text();
            db_.rollback();
            return false;
        }

        shapeIdMap.insert(index, query.value(0).toInt());
    }

    for (const Connection& connection : drawing.connections()) {
        int fromIndex = connection.getFromShapeId();
        int toIndex = connection.getToShapeId();
        if (!shapeIdMap.contains(fromIndex) || !shapeIdMap.contains(toIndex)) {
            continue;
        }

        query.prepare(R"(
            INSERT INTO connections (drawing_id, from_shape_id, to_shape_id)
            VALUES (:drawing_id, :from_shape_id, :to_shape_id)
        )");
        query.bindValue(":drawing_id", drawingId);
        query.bindValue(":from_shape_id", shapeIdMap.value(fromIndex));
        query.bindValue(":to_shape_id", shapeIdMap.value(toIndex));
        if (!query.exec()) {
            error = query.lastError().text();
            db_.rollback();
            return false;
        }
    }

    if (!db_.commit()) {
        error = db_.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::loadDrawing(int drawingId, Drawing& drawing, QString& error) {
    if (!db_.isOpen()) {
        error = "Database connection is not open.";
        return false;
    }

    QSqlQuery query(db_);
    query.prepare(R"(
        SELECT id, type, start_x, start_y, end_x, end_y
        FROM shapes
        WHERE drawing_id = :drawing_id
        ORDER BY shape_order ASC
    )");
    query.bindValue(":drawing_id", drawingId);
    if (!query.exec()) {
        error = query.lastError().text();
        return false;
    }

    QHash<int, int> dbIdToIndex;
    int loadIndex = 0;
    while (query.next()) {
        int shapeDbId = query.value(0).toInt();
        Shape::Type type = static_cast<Shape::Type>(query.value(1).toInt());
        QPoint start(query.value(2).toInt(), query.value(3).toInt());
        QPoint end(query.value(4).toInt(), query.value(5).toInt());

        auto shape = Shape::createFromType(type);
        if (!shape) {
            error = "Unsupported shape type in database.";
            return false;
        }
        shape->setStart(start);
        shape->setEnd(end);
        drawing.addShape(std::move(shape));
        dbIdToIndex.insert(shapeDbId, loadIndex);
        loadIndex++;
    }

    query.prepare(R"(
        SELECT from_shape_id, to_shape_id
        FROM connections
        WHERE drawing_id = :drawing_id
    )");
    query.bindValue(":drawing_id", drawingId);
    if (!query.exec()) {
        error = query.lastError().text();
        return false;
    }

    while (query.next()) {
        int fromDbId = query.value(0).toInt();
        int toDbId = query.value(1).toInt();
        if (!dbIdToIndex.contains(fromDbId) || !dbIdToIndex.contains(toDbId)) {
            continue;
        }
        drawing.connections().emplace_back(dbIdToIndex.value(fromDbId), dbIdToIndex.value(toDbId));
    }

    return true;
}

bool DatabaseManager::loadLatestDrawing(Drawing& drawing, QString& error) {
    if (!db_.isOpen()) {
        error = "Database connection is not open.";
        return false;
    }

    QSqlQuery query(db_);
    if (!query.exec("SELECT id FROM drawings ORDER BY created_at DESC LIMIT 1")) {
        error = query.lastError().text();
        return false;
    }

    if (!query.next()) {
        error = "No drawings found in database.";
        return false;
    }

    int drawingId = query.value(0).toInt();
    return loadDrawing(drawingId, drawing, error);
}
