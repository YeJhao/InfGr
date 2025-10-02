#include "sphere.hpp"
#include <stdexcept>

using namespace std;

Sphere::Sphere(const Point& center_, double radius_) : center(center_), radius(radius_) 
{
    if (radius <= 0) {
        throw invalid_argument("El radio de la esfera debe ser positivo.");
    }
}