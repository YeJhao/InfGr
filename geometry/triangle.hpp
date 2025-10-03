#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include "geometry.hpp"
#include "geometric_shape.hpp"

class Triangle : public GeometricShape {
    public:
        Point v0, v1, v2; // The three vertices of the triangle
        Direction normal; // Normal vector (computed from vertices)
        
        Triangle(const Point& vertex0, const Point& vertex1, const Point& vertex2);
        
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