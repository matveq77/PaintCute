#include "Connection.h"
#include <QPainter>
#include <QPoint>
#include <cmath>

Connection::Connection()
    : fromShapeId_(-1), toShapeId_(-1) {}

Connection::Connection(int fromShapeId, int toShapeId)
    : fromShapeId_(fromShapeId), toShapeId_(toShapeId) {}

void Connection::draw(QPainter& painter, const QPoint& fromCenter, const QPoint& toCenter) const {
    
    painter.drawLine(fromCenter, toCenter);

    double angle = std::atan2(toCenter.y() - fromCenter.y(),
        toCenter.x() - fromCenter.x());

    QPoint arrowP1 = toCenter - QPoint(std::cos(angle - M_PI / 6) * ARROW_SIZE,
        std::sin(angle - M_PI / 6) * ARROW_SIZE);
    QPoint arrowP2 = toCenter - QPoint(std::cos(angle + M_PI / 6) * ARROW_SIZE,
        std::sin(angle + M_PI / 6) * ARROW_SIZE);

    painter.drawLine(toCenter, arrowP1);
    painter.drawLine(toCenter, arrowP2);
}

void Connection::serialize(QDataStream& out) const {
    out << fromShapeId_ << toShapeId_;
}

void Connection::deserialize(QDataStream& in) {
    in >> fromShapeId_ >> toShapeId_;
}
