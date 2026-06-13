#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QFileDialog>
#include <QVector>
#include <QDataStream>
#include <QFile>
#include <QCursor>
#include <QKeyEvent>
#include <QMessageBox>  
#include <memory>

#include "Shape.h"
#include "Rectangle.h"
#include "Ellipse.h"
#include "Triangle.h"
#include "Connection.h"
#include "DrawingSerializer.h"

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
        //setStyleSheet("background-color: white;");
    }

    ~DrawingArea() {
        for (auto& shape : shapes_) {
            delete shape;
        }
    }

    void setMode(Mode mode) {
        mode_ = mode;
        connectionStartIndex_ = -1;
        isDrawing_ = false;
        drawingShape_.reset();
        selectedShapeIndex_ = -1;
        setMouseTracking(false);
        update();
    }

    void clearAll() {
        for (auto& shape : shapes_) {
            delete shape;
        }
        shapes_.clear();
        connections_.clear();
        isDrawing_ = false;
        selectedShapeIndex_ = -1;
        update();
    }

    void saveToFile(const QString& filename) {
        auto result = DrawingSerializer::save(filename, shapes_, connections_);
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
        QVector<Shape*> newShapes;
        QVector<Connection> newConnections;

        auto result = DrawingSerializer::load(filename, newShapes, newConnections);
        if (result == DrawingSerializer::Result::FileOpenError) {
            QMessageBox::warning(this, "Error", "Cannot open file!");
            return;
        }
        if (result != DrawingSerializer::Result::Ok) {
            for (auto* s : newShapes) delete s;
            QMessageBox::warning(this, "Error", "Invalid or corrupted file!");
            return;
        }

        clearAll();
        shapes_ = newShapes;
        connections_ = newConnections;
        update();
        QMessageBox::information(this, "Success", "Drawing loaded!", QMessageBox::Ok);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), Qt::white);

        painter.setPen(QPen(Qt::black, 2));
        for (int i = 0; i < shapes_.size(); ++i) {
            if (mode_ == Mode::Move && i == selectedShapeIndex_) {
                painter.setPen(QPen(Qt::blue, 2, Qt::DashLine));
            }
            else {
                painter.setPen(QPen(Qt::black, 2));
            }
            shapes_[i]->draw(painter);
        }

        painter.setPen(QPen(Qt::gray, 1.5));
        for (const auto& conn : connections_) {
            if (conn.getFromShapeId() >= 0 && conn.getFromShapeId() < shapes_.size() &&
                conn.getToShapeId() >= 0 && conn.getToShapeId() < shapes_.size()) {
                QPoint fromCenter = shapes_[conn.getFromShapeId()]->getCenter();
                QPoint toCenter = shapes_[conn.getToShapeId()]->getCenter();
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
            QPoint fromCenter = shapes_[connectionStartIndex_]->getCenter();
            painter.drawLine(fromCenter, lastMousePos_);
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
            shapes_[selectedShapeIndex_]->move(delta);
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
    QVector<Shape*> shapes_;
    QVector<Connection> connections_;
    Mode mode_;
    std::unique_ptr<Shape> drawingShape_;
    int selectedShapeIndex_;
    int connectionStartIndex_;
    QPoint startPoint_;
    QPoint lastMousePos_;
    bool isDrawing_;

    void handleLeftMousePress(const QPoint& pos) {
        lastMousePos_ = pos;

        switch (mode_) {
        case Mode::DrawRectangle:
        case Mode::DrawEllipse:
        case Mode::DrawTriangle:
            if (!isDrawing_) {
                startPoint_ = pos;
                Shape::Type type = (mode_ == Mode::DrawRectangle) ? Shape::Type::Rectangle :
                    (mode_ == Mode::DrawEllipse) ? Shape::Type::Ellipse :
                    Shape::Type::Triangle;
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
                    connections_.append(Connection(connectionStartIndex_, shapeIndex));
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
                    shapes_.append(drawingShape_.release());
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
    }

    int getShapeAtPoint(const QPoint& pos) {
        for (int i = shapes_.size() - 1; i >= 0; --i) {
            if (shapes_[i]->isPointInShape(pos)) {
                return i;
            }
        }
        return -1;
    }

    void deleteShape(int index) {
        if (index < 0 || index >= shapes_.size()) return;

        delete shapes_[index];
        shapes_.removeAt(index);

        QVector<Connection> validConnections;
        for (const auto& conn : connections_) {
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
            validConnections.append(newConn);
        }
        connections_ = validConnections;

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

    controlLayout->addWidget(btnSave);
    controlLayout->addWidget(btnLoad);
    controlLayout->addWidget(btnClear);
    controlLayout->addStretch();

    DrawingArea* drawingArea = new DrawingArea();

    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(drawingArea);

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

    window.show();
    return app.exec();
}

#include "main.moc"