#include "sphere.hpp"
#include "../ray.hpp"
#include <stdexcept>
#include <cmath>
#include <iostream>

using namespace std;

Sphere::Sphere(const Point& center_, double radius_, const Color& emission_, 
               const Color& kd_, const Color& ks_, const Color& kt_)
    : center(center_), radius(radius_), emission(emission_), kd(kd_), ks(ks_), kt(kt_)
{
    // Asegura que los valores sean positivos
    if (emission.r < 0 || emission.g < 0 || emission.b < 0) {
        throw invalid_argument("Los valores de emision deben ser no negativos");
    }

    if (kd.r < 0 || kd.g < 0 || kd.b < 0 || 
        ks.r < 0 || ks.g < 0 || ks.b < 0 || 
        kt.r < 0 || kt.g < 0 || kt.b < 0) {
        throw invalid_argument("Los coeficientes de color deben ser no negativos");
    }
    
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
        
        // Only add intersections with positive t (in front of ray origin)
        if (t1 > 1e-6) {  // Use small epsilon to avoid self-intersection
            intersectionPoints.push_back(ray.o + ray.d * t1);
        }
        if (t2 > 1e-6 && abs(t2 - t1) > 1e-6) {  // Avoid duplicate points
            intersectionPoints.push_back(ray.o + ray.d * t2);
        }
        
        // Sort intersections by distance from ray origin (closest first)
        if (intersectionPoints.size() == 2) {
            double dist1 = (intersectionPoints[0] - ray.o).norm();
            double dist2 = (intersectionPoints[1] - ray.o).norm();
            if (dist1 > dist2) {
                swap(intersectionPoints[0], intersectionPoints[1]);
            }
        }
        
        return intersectionPoints;
    }
}

void Sphere::print() const {
    cout << "Sphere:\n  center=" << center << ", radius=" << radius << endl;
}

Direction Sphere::calculateNormalAtPoint(const Point& p) const {
    return (p - center).normalized();
}