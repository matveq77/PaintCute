#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QFileDialog>
#include <QDateTime>
#include <QLabel>
#include <QInputDialog>
//#include <QMap>
#include <QDataStream>
#include <vector>
#include <QFile>
#include <QCursor>
#include <QKeyEvent>
#include <QMessageBox>  
#include <memory>

#include "Drawing.h"
#include "Shape.h"
#include "Rectangle.h"
#include "Ellipse.h"
#include "Triangle.h"
#include "Connection.h"
#include "DrawingSerializer.h"
#include "DatabaseManager.h"

class DrawingArea : public QWidget {
    Q_OBJECT

public:
    enum class Mode {
        None,
        DrawRectangle,
        DrawEllipse,
        DrawTriangle,
        DrawConnection,
        Move,
        Delete
    };

    explicit DrawingArea(QWidget* parent = nullptr)
        : QWidget(parent), mode_(Mode::None), drawingShape_(nullptr),
        selectedShapeIndex_(-1), connectionStartIndex_(-1), isDrawing_(false) {
        setFocusPolicy(Qt::StrongFocus);
    }

    void setMode(Mode mode) {
        mode_ = mode;
        connectionStartIndex_ = -1;
        isDrawing_ = false;
        drawingShape_.reset();
        if (selectedShapeIndex_ >= 0) {
            releaseMouse();
        }
        selectedShapeIndex_ = -1;
        setMouseTracking(false);
        setCursor(Qt::ArrowCursor);
        update();
    }

    void clearAll() {
        drawing_.clear();
        isDrawing_ = false;
        selectedShapeIndex_ = -1;
        update();
    }

    void saveToFile(const QString& filename) {
        auto result = DrawingSerializer::save(filename, drawing_);
        if (result == DrawingSerializer::Result::FileOpenError) {
            QMessageBox::warning(this, "Error", "Cannot create file!");
            return;
        }
        if (result != DrawingSerializer::Result::Ok) {
            QMessageBox::warning(this, "Error", "Failed to write drawing!");
            return;
        }
        QMessageBox::information(this, "Success", "Drawing saved!");
    }

    void loadFromFile(const QString& filename) {
        Drawing newDrawing;
        auto result = DrawingSerializer::load(filename, newDrawing);
        if (result == DrawingSerializer::Result::FileOpenError) {
            QMessageBox::warning(this, "Error", "Cannot open file!");
            return;
        }
        if (result != DrawingSerializer::Result::Ok) {
            QMessageBox::warning(this, "Error", "Invalid or corrupted file!");
            return;
        }

        drawing_ = std::move(newDrawing);
        isDrawing_ = false;
        selectedShapeIndex_ = -1;
        update();
        QMessageBox::information(this, "Success", "Drawing loaded!", QMessageBox::Ok);
    }

    const Drawing& drawing() const { return drawing_; }
    void setDrawing(Drawing drawing) {
        drawing_ = std::move(drawing);
        isDrawing_ = false;
        selectedShapeIndex_ = -1;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), Qt::white);

        auto& connections = drawing_.connections();
        int shapeCount = drawing_.shapeCount();

        for (int i = 0; i < shapeCount; ++i) {
            auto* s = drawing_.shapeAt(i);
            if (!s) continue;
            if (mode_ == Mode::Move && i == selectedShapeIndex_) {
                painter.setPen(QPen(Qt::blue, 2, Qt::DashLine));
            }
            else {
                painter.setPen(QPen(Qt::black, 2));
            }
            s->draw(painter);
        }

        painter.setPen(QPen(Qt::gray, 1.5));
        for (const auto& conn : connections) {
            int fromId = conn.getFromShapeId();
            int toId = conn.getToShapeId();
            if (fromId >= 0 && fromId < shapeCount && toId >= 0 && toId < shapeCount) {
                auto* fromShape = drawing_.shapeAt(fromId);
                auto* toShape = drawing_.shapeAt(toId);
                if (!fromShape || !toShape) continue;
                QPoint fromCenter = fromShape->getCenter();
                QPoint toCenter = toShape->getCenter();
                conn.draw(painter, fromCenter, toCenter);
            }
        }

        if (isDrawing_ && mode_ != Mode::DrawConnection) {
            painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
            if (drawingShape_) {
                drawingShape_->draw(painter);
            }
        }

        if (mode_ == Mode::DrawConnection && connectionStartIndex_ >= 0 && isDrawing_) {
            painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
            if (auto* fromShape = drawing_.shapeAt(connectionStartIndex_)) {
                QPoint fromCenter = fromShape->getCenter();
                painter.drawLine(fromCenter, lastMousePos_);
            }
        }
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            handleLeftMousePress(event->pos());
        }
        else if (event->button() == Qt::RightButton) {
            handleRightMousePress();
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (mode_ == Mode::Move && selectedShapeIndex_ >= 0 && (event->buttons() & Qt::LeftButton)) {
            setCursor(Qt::OpenHandCursor);
            QPoint delta = event->pos() - lastMousePos_;
            if (auto* s = drawing_.shapeAt(selectedShapeIndex_)) s->move(delta);
            lastMousePos_ = event->pos();
            update();
        }
        else if (mode_ == Mode::Move && selectedShapeIndex_ >= 0 && !(event->buttons() & Qt::LeftButton)) {
            selectedShapeIndex_ = -1;
            setCursor(Qt::ArrowCursor);
            update();
        }
        else if (isDrawing_ && mode_ != Mode::DrawConnection && mode_ != Mode::Move) {
            if (drawingShape_) {
                drawingShape_->setEnd(event->pos());
            }
            update();
        }
        else if (mode_ == Mode::DrawConnection && connectionStartIndex_ >= 0 && isDrawing_) {
            lastMousePos_ = event->pos();
            update();
        }
        else {
            setCursor(Qt::ArrowCursor);
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            handleLeftMouseRelease(event->pos());
        }
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape) {
            cancelCurrentAction();
        }
        QWidget::keyPressEvent(event);
    }

private:
    Drawing drawing_;
    Mode mode_;
    std::unique_ptr<Shape> drawingShape_;
    int selectedShapeIndex_;
    int connectionStartIndex_;
    QPoint startPoint_;
    QPoint lastMousePos_;
    bool isDrawing_;

    static Shape::Type modeToType(Mode mode) {
        switch (mode) {
        case Mode::DrawRectangle: return Shape::Type::Rectangle;
        case Mode::DrawEllipse:   return Shape::Type::Ellipse;
        case Mode::DrawTriangle:  return Shape::Type::Triangle;
        default: return Shape::Type::Rectangle;
        }
    }

    void handleLeftMousePress(const QPoint& pos) {
        lastMousePos_ = pos;

        switch (mode_) {
        case Mode::DrawRectangle:
        case Mode::DrawEllipse:
        case Mode::DrawTriangle:
            if (!isDrawing_) {
                startPoint_ = pos;
                Shape::Type type = modeToType(mode_);
                drawingShape_ = Shape::createFromType(type);
                drawingShape_->setStart(pos);
                drawingShape_->setEnd(pos);
                isDrawing_ = true;
                update();
            }
            break;

        case Mode::DrawConnection: {
            int shapeIndex = getShapeAtPoint(pos);
            if (connectionStartIndex_ < 0) {
                if (shapeIndex >= 0) {
                    connectionStartIndex_ = shapeIndex;
                    isDrawing_ = true;
                    setMouseTracking(true);
                }
            }
            else {
                if (shapeIndex >= 0 && shapeIndex != connectionStartIndex_) {
                    drawing_.connections().push_back(Connection(connectionStartIndex_, shapeIndex));
                }
                connectionStartIndex_ = -1;
                isDrawing_ = false;
                setMouseTracking(false);
                update();
            }
            break;
        }

        case Mode::Move: {
            int shapeIndex = getShapeAtPoint(pos);
            if (shapeIndex >= 0) {
                selectedShapeIndex_ = shapeIndex;
                grabMouse();
                setCursor(Qt::OpenHandCursor);
            }
            break;
        }

        case Mode::Delete: {
            int shapeIndex = getShapeAtPoint(pos);
            if (shapeIndex >= 0) {
                deleteShape(shapeIndex);
            }
            break;
        }

        default:
            break;
        }
    }

    void handleLeftMouseRelease(const QPoint& pos) {
        if (mode_ == Mode::DrawRectangle || mode_ == Mode::DrawEllipse ||
            mode_ == Mode::DrawTriangle) {
            if (isDrawing_) {
                drawingShape_->setEnd(pos);
                QRect bounds = drawingShape_->getBoundingRect();
                if (bounds.width() > 5 && bounds.height() > 5) {
                    drawing_.addShape(std::move(drawingShape_));
                }
                else {
                    drawingShape_.reset();
                }
                isDrawing_ = false;
                update();
            }
        }
        else if (mode_ == Mode::Move) {
            selectedShapeIndex_ = -1;
            releaseMouse();
            setCursor(Qt::ArrowCursor);
            update();
        }
    }

    void handleRightMousePress() {
        cancelCurrentAction();
    }

    void cancelCurrentAction() {
        if (isDrawing_) {
            drawingShape_.reset();
            connectionStartIndex_ = -1;
            isDrawing_ = false;
            setMouseTracking(false);
            releaseMouse();
            update();
        }
        if (mode_ == Mode::Move && selectedShapeIndex_ >= 0) {
            selectedShapeIndex_ = -1;
            releaseMouse();
            setCursor(Qt::ArrowCursor);
            update();
        }
    }

    int getShapeAtPoint(const QPoint& pos) {
        for (int i = drawing_.shapeCount() - 1; i >= 0; --i) {
            if (auto* s = drawing_.shapeAt(i)) {
                if (s->isPointInShape(pos)) {
                    return i;
                }
            }
        }
        return -1;
    }

    void deleteShape(int index) {
        auto& connections = drawing_.connections();

        if (index < 0 || index >= drawing_.shapeCount()) return;

        drawing_.removeShape(index);

        std::vector<Connection> validConnections;
        for (const auto& conn : connections) {
            int from = conn.getFromShapeId();
            int to = conn.getToShapeId();
            if (from == index || to == index) {
                continue;
            }

            Connection newConn = conn;
            if (from > index) {
                newConn.setFromShapeId(from - 1);
            }
            if (to > index) {
                newConn.setToShapeId(to - 1);
            }
            validConnections.push_back(newConn);
        }
        connections = validConnections;

        update();
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Drawing Application - Painton");
    window.resize(1200, 800);

    QVBoxLayout* mainLayout = new QVBoxLayout(&window);

    QHBoxLayout* controlLayout = new QHBoxLayout();

    QPushButton* btnRectangle = new QPushButton("Rectangle");
    QPushButton* btnEllipse = new QPushButton("Ellipse");
    QPushButton* btnTriangle = new QPushButton("Triangle");
    QPushButton* btnConnection = new QPushButton("Connection");
    QPushButton* btnMove = new QPushButton("Move");
    QPushButton* btnDelete = new QPushButton("Delete");

    controlLayout->addWidget(btnRectangle);
    controlLayout->addWidget(btnEllipse);
    controlLayout->addWidget(btnTriangle);
    controlLayout->addWidget(btnConnection);
    controlLayout->addWidget(btnMove);
    controlLayout->addWidget(btnDelete);
    controlLayout->addSpacing(20);

    QPushButton* btnSave = new QPushButton("Save");
    QPushButton* btnLoad = new QPushButton("Load");
    QPushButton* btnClear = new QPushButton("Clear All");
    QPushButton* btnDbSave = new QPushButton("Save to DB");
    QPushButton* btnDbLoad = new QPushButton("Load from DB");
    QLabel* dbStatusLabel = new QLabel("DB: disconnected");

    dbStatusLabel->setStyleSheet("color: #333; font-weight: bold; padding: 2px 6px;");
    dbStatusLabel->setAlignment(Qt::AlignCenter);

    controlLayout->addWidget(btnSave);
    controlLayout->addWidget(btnLoad);
    controlLayout->addWidget(btnClear);
    controlLayout->addWidget(btnDbSave);
    controlLayout->addWidget(btnDbLoad);
    controlLayout->addWidget(dbStatusLabel);
    controlLayout->addStretch();

    DrawingArea* drawingArea = new DrawingArea();

    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(drawingArea);

    DatabaseManager* dbManager = new DatabaseManager(&window);
    auto updateDbStatus = [&](bool connected) {
        dbStatusLabel->setText(connected ? "DB: connected" : "DB: disconnected");
    };

    QObject::connect(btnRectangle, &QPushButton::clicked,
        [drawingArea]() { drawingArea->setMode(DrawingArea::Mode::DrawRectangle); });
    QObject::connect(btnEllipse, &QPushButton::clicked,
        [drawingArea]() { drawingArea->setMode(DrawingArea::Mode::DrawEllipse); });
    QObject::connect(btnTriangle, &QPushButton::clicked,
        [drawingArea]() { drawingArea->setMode(DrawingArea::Mode::DrawTriangle); });
    QObject::connect(btnConnection, &QPushButton::clicked,
        [drawingArea]() { drawingArea->setMode(DrawingArea::Mode::DrawConnection); });
    QObject::connect(btnMove, &QPushButton::clicked,
        [drawingArea]() { drawingArea->setMode(DrawingArea::Mode::Move); });
    QObject::connect(btnDelete, &QPushButton::clicked,
        [drawingArea]() { drawingArea->setMode(DrawingArea::Mode::Delete); });
    QObject::connect(btnClear, &QPushButton::clicked,
        [drawingArea]() { drawingArea->clearAll(); });

    QObject::connect(btnSave, &QPushButton::clicked, [&]() {
        QString filename = QFileDialog::getSaveFileName(&window, "Save Drawing", "",
            "Drawing Files (*.pnt);;All Files (*)");
        if (!filename.isEmpty()) {
            drawingArea->saveToFile(filename);
        }
        });

    QObject::connect(btnLoad, &QPushButton::clicked, [&]() {
        QString filename = QFileDialog::getOpenFileName(&window, "Load Drawing", "",
            "Drawing Files (*.pnt);;All Files (*)");
        if (!filename.isEmpty()) {
            drawingArea->loadFromFile(filename);
        }
        });

    QObject::connect(btnDbConnect, &QPushButton::clicked, [&]() {
        const QString host = "localhost";
        const int port = 5432;
        const QString database = "paintcute";
        const QString user = "postgres";
        const QString password = "postgres";

        QString error;
        if (!dbManager->open(host, port, database, user, password, error)) {
            QMessageBox::warning(&window, "Database Error", "Unable to connect to PostgreSQL:\n" + error);
            updateDbStatus(false);
            return;
        }
        QMessageBox::information(&window, "Database", "Connected to PostgreSQL successfully.");
        updateDbStatus(true);
    });

    QObject::connect(btnDbSave, &QPushButton::clicked, [&]() {
        if (!dbManager->isOpen()) {
            QMessageBox::warning(&window, "Database", "Please connect to the database first.");
            return;
        }

        QString drawingName = QString("Drawing %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

        QString error;
        if (!dbManager->saveDrawing(drawingName, drawingArea->drawing(), error)) {
            QMessageBox::warning(&window, "Database", "Failed to save drawing:\n" + error);
            return;
        }
        QMessageBox::information(&window, "Database", "Drawing saved to PostgreSQL as " + drawingName + ".");
    });

    QObject::connect(btnDbLoad, &QPushButton::clicked, [&]() {
        if (!dbManager->isOpen()) {
            QMessageBox::warning(&window, "Database", "Please connect to the database first.");
            return;
        }

        QString error;
        Drawing loaded;
        if (!dbManager->loadLatestDrawing(loaded, error)) {
            QMessageBox::warning(&window, "Database", "Failed to load drawing:\n" + error);
            return;
        }

        drawingArea->setDrawing(std::move(loaded));
        QMessageBox::information(&window, "Database", "Latest drawing loaded from PostgreSQL.");
    });

    window.show();
    return app.exec();
}

#include "main.moc"