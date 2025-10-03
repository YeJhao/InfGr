#ifndef PLANE_HPP
#define PLANE_HPP

#include "geometry.hpp"
#include "geometric_shape.hpp"

class Plane : public GeometricShape {
    public:
        Direction normal;
        Point origin;
        
        Plane(const Direction& normal_, const Point& point_);
        
        // Override the pure virtual method
        std::vector<Point> intersections(const Ray& ray) const override;
        
        // Override the print method
        void print() const override;
};

#endif // PLANE_HPP