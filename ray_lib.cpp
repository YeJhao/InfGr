#include "ray.hpp"
#include "geometry/geometric_shape.hpp"

// Ray implementations
Ray::Ray(const Point& origin_, const Direction& direction_) : o(origin_), d(direction_) {}

// New generic method that works with any geometric shape
std::vector<Point> Ray::intersections(const GeometricShape& shape) const {
    return shape.intersections(*this);
}