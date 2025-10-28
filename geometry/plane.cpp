#include "plane.hpp"
#include "../ray.hpp"
#include <cmath>
#include <iostream>

using namespace std;

Plane::Plane(const Direction& normal_, double distance_, const Color& emission_, 
             const Color& kd_, const Color& ks_, const Color& kt_)
    : normal(normal_.normalized()), distance(distance_), emission(emission_), 
      kd(kd_), ks(ks_), kt(kt_)
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
}

std::vector<Point> Plane::intersections(const Ray& ray) const {
    vector<Point> intersectionPoints;
    
    double denom = ray.d.dot(normal);
    if (fabs(denom) > 1e-6) { // Ensure the ray is not parallel to the plane
        // Ecuación del plano: normal · P + distance = 0
        // Sustituir P = ray.o + t * ray.d
        // normal · (ray.o + t * ray.d) + distance = 0
        // normal · ray.o + t * (normal · ray.d) + distance = 0
        // t = -(normal · ray.o + distance) / (normal · ray.d)
        double t = -(normal.dot(Direction(ray.o.coords)) + distance) / denom;
        if (t >= 0) {
            Point intersection = ray.o + ray.d * t;
            intersectionPoints.push_back(intersection);
        }
    }
    
    return intersectionPoints;
}

void Plane::print() const {
    cout << "Plane:\n  normal=" << normal << ", distance=" << distance << 
    ", emission=" << emission.r << ", " << emission.g << ", " << emission.b << 
    ", kd=" << kd.r << ", " << kd.g << ", " << kd.b << 
    ", ks=" << ks.r << ", " << ks.g << ", " << ks.b << 
    ", kt=" << kt.r << ", " << kt.g << ", " << kt.b << endl;
}