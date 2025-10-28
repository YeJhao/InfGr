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
        Color kd, ks, kt;

        Sphere(const Point& center_, double radius_, const Color& emission_, 
               const Color& kd_, const Color& ks_, const Color& kt_);

        // Override the pure virtual method
        std::vector<Point> intersections(const Ray& ray) const override;
        
        // Calculate normal at a given point on the sphere surface
        Direction calculateNormalAtPoint(const Point& p) const;
        
        // Override the print method
        void print() const override;
};

#endif // SPHERE_HPP