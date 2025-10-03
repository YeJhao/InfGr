#ifndef GEOMETRIC_SHAPE_HPP
#define GEOMETRIC_SHAPE_HPP

#include "geometry.hpp" 
#include <vector>

// Forward declaration
class Ray;

class GeometricShape {
public:
    virtual ~GeometricShape() = default;
    
    // Pure virtual method that each shape must implement
    virtual std::vector<Point> intersections(const Ray& ray) const = 0;
    
    // Optional: virtual method for displaying shape info
    virtual void print() const = 0;
};

#endif // GEOMETRIC_SHAPE_HPP