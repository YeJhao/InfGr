#include "sphere.hpp"
#include "../ray.hpp"
#include <stdexcept>
#include <cmath>
#include <iostream>

using namespace std;

Sphere::Sphere(const Point& center_, double radius_) : center(center_), radius(radius_) 
{
    if (radius <= 0) {
        throw invalid_argument("El radio de la esfera debe ser positivo.");
    }
}

std::vector<Point> Sphere::intersections(const Ray& ray) const {
    vector<Point> intersectionPoints;
    Direction oc = ray.o - center;
    double a = ray.d.dot(ray.d);
    double b = 2.0 * oc.dot(ray.d);
    double c = oc.dot(oc) - radius * radius;
    double discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return intersectionPoints; // No intersection, empty vector
    } else {
        double t1 = (-b - sqrt(discriminant)) / (2.0 * a);
        double t2 = (-b + sqrt(discriminant)) / (2.0 * a);
        
        if (t1 >= 0) {
            intersectionPoints.push_back(ray.o + ray.d * t1);
        }
        if (t2 >= 0 && t2 != t1) {
            intersectionPoints.push_back(ray.o + ray.d * t2);
        }
        
        // Sort intersections so closest is first
        if (intersectionPoints.size() == 2 && t1 > t2) {
            swap(intersectionPoints[0], intersectionPoints[1]);
        }
        
        return intersectionPoints;
    }
}

void Sphere::print() const {
    cout << "Sphere: center=" << center << ", radius=" << radius << endl;
}