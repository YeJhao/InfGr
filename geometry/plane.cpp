#include "plane.hpp"
#include "../ray.hpp"
#include <cmath>
#include <iostream>

using namespace std;

Plane::Plane(const Direction& normal_, const Point& point_) : normal(normal_), origin(point_) {}

std::vector<Point> Plane::intersections(const Ray& ray) const {
    vector<Point> intersectionPoints;
    
    double denom = ray.d.dot(normal);
    if (fabs(denom) > 1e-6) { // Ensure the ray is not parallel to the plane
        double t = (origin - ray.o).dot(normal) / denom;
        if (t >= 0) {
            intersectionPoints.push_back(ray.o + ray.d * t);
        }
    }
    
    return intersectionPoints;
}

void Plane::print() const {
    cout << "Plane: normal=" << normal << ", point=" << origin << endl;
}