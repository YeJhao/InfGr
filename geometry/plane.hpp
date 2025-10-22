#ifndef PLANE_HPP
#define PLANE_HPP

#include "geometry.hpp"
#include "geometric_shape.hpp"
#include "color.hpp"

class Plane : public GeometricShape {
    public:
        Direction normal;
        double distance; // Distancia con signo desde el origen al plano
        Color emission;
        
        Plane(const Direction& normal_, double distance_, const Color& emission_);
        
        // Override the pure virtual method
        std::vector<Point> intersections(const Ray& ray) const override;
        
        // Override the print method
        void print() const override;
};

#endif // PLANE_HPP