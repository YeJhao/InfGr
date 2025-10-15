#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "geometry.hpp"
#include "geometric_shape.hpp"
#include "color.hpp"

class Sphere : public GeometricShape {
    public: 
        Point center;
        double radius;
        Color emission;

        Sphere(const Point& center_, double radius_, const Color& emission_);
        
        // Override the pure virtual method
        std::vector<Point> intersections(const Ray& ray) const override;
        
        // Override the print method
        void print() const override;
};

#endif // SPHERE_HPP