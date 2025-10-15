#include "plane.hpp"
#include "../ray.hpp"
#include <cmath>
#include <iostream>

using namespace std;

Plane::Plane(const Direction& normal_, const Point& point_, const Color& emission_)
    : normal(normal_), origin(point_), emission(emission_)
{
    // Asegura que los valores sean positivos
    if (emission.r < 0 || emission.g < 0 || emission.b < 0) {
        throw invalid_argument("Los valores de emision deben ser no negativos");
    }
}

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