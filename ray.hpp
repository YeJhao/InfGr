#ifndef RAY_HPP
#define RAY_HPP

#include "geometry/geometry.hpp"
#include "geometry/sphere.hpp"
#include "geometry/plane.hpp"
#include <vector>

class Ray {
    public:
        Point o; // Origin of the ray
        Direction d; // Direction of the ray (should be normalized)

        Ray(const Point& origin_, const Direction& direction_);

        std::vector<Point> sphereIntersections(const Sphere& sphere);
        Point planeIntersection(const Plane& plane);
};

#endif // RAY_HPP