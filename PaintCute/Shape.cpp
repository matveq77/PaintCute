#include "Shape.h"
#include "Rectangle.h"
#include "Ellipse.h"
#include "Triangle.h"

Shape::Shape(Type type) : type_(type), start_(0, 0), end_(0, 0) {}

std::unique_ptr<Shape> Shape::createFromType(Type type) {
    switch (type) {
    case Type::Rectangle: return std::make_unique<Rectangle>();
    case Type::Ellipse: return std::make_unique<Ellipse>();
    case Type::Triangle: return std::make_unique<Triangle>();
    default: return nullptr;
    }
}
