#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include "geometry.hpp"
#include "geometric_shape.hpp"
#include "color.hpp"

class Triangle : public GeometricShape {
    public:
        Point v0, v1, v2; // The three vertices of the triangle
        Direction normal; // Normal vector (computed from vertices)
        Color emission;  // Emission color of the triangle
        CoefficientColor coefficient; // Coefficients for the triangle

        Triangle(const Point& vertex0, const Point& vertex1, const Point& vertex2, const Color& emission_, const CoefficientColor& coefficient_);

        // Override the pure virtual method
        std::vector<Point> intersections(const Ray& ray) const override;
        
        // Override the print method
        void print() const override;
        
    private:
        // Helper method to compute normal from vertices
        void computeNormal();
        
        // Helper method to check if a point is inside the triangle
        bool isPointInside(const Point& p) const;
};

#endif // TRIANGLE_HPP