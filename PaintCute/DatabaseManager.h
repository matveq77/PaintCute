#pragma once

#include <QObject>
#include <QList>
#include <QPair>
#include <QString>
#include <QSqlDatabase>

class Drawing;

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager() override;

    bool open(const QString& host, int port, const QString& dbName,
              const QString& user, const QString& password,
              QString& error);

    bool isOpen() const;
    bool saveDrawing(const QString& name, const Drawing& drawing, QString& error);
    bool loadDrawing(int drawingId, Drawing& drawing, QString& error);
    bool loadLatestDrawing(Drawing& drawing, QString& error);
    QList<QPair<int, QString>> listDrawings(QString& error);

private:
    bool ensureSchema(QString& error);

    QSqlDatabase db_;
    QString lastError_;
};
