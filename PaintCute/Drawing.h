#pragma once

#include <vector>
#include <QDataStream>
#include "Shape.h"
#include "Connection.h"
#include <memory>

class Drawing {
public:
    Drawing() = default;
    ~Drawing();

    Drawing(const Drawing&) = delete;
    Drawing& operator=(const Drawing&) = delete;
    Drawing(Drawing&&) = default;
    Drawing& operator=(Drawing&&) = default;

    void clear();

    std::vector<std::unique_ptr<Shape>>& shapes() { return shapes_; }
    const std::vector<std::unique_ptr<Shape>>& shapes() const { return shapes_; }

    int shapeCount() const { return static_cast<int>(shapes_.size()); }
    Shape* shapeAt(int i) { return (i >= 0 && i < static_cast<int>(shapes_.size())) ? shapes_[i].get() : nullptr; }
    const Shape* shapeAt(int i) const { return (i >= 0 && i < static_cast<int>(shapes_.size())) ? shapes_[i].get() : nullptr; }

    void addShape(std::unique_ptr<Shape> s) { shapes_.push_back(std::move(s)); }
    void removeShape(int i) { if (i >= 0 && i < static_cast<int>(shapes_.size())) shapes_.erase(shapes_.begin() + i); }

    std::vector<Connection>& connections() { return connections_; }
    const std::vector<Connection>& connections() const { return connections_; }

    bool serialize(QDataStream& out) const;
    bool deserialize(QDataStream& in);

private:
    std::vector<std::unique_ptr<Shape>> shapes_;
    std::vector<Connection> connections_;
};