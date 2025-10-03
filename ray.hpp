#ifndef RAY_HPP
#define RAY_HPP

#include "geometry/geometry.hpp"
#include <vector>

// Forward declaration
class GeometricShape;

class Ray {
    public:
        Point o; // Origin of the ray
        Direction d; // Direction of the ray (should be normalized)

        Ray(const Point& origin_, const Direction& direction_);

        // Generic method that works with any geometric shape
        std::vector<Point> intersections(const GeometricShape& shape) const;
};

#endif // RAY_HPP