/*
* ray_lib.cpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la implementación de la clase Ray,
* que representa un rayo en la escena.
*/

#include "ray.hpp"
#include "geometry/geometric_shape.hpp"

// Constructor de Ray
Ray::Ray(const Point& origin_, const Direction& direction_) : o(origin_), d(direction_) {}

// Implementación de el método intersections
std::vector<Point> Ray::intersections(const GeometricShape& shape) const {
    return shape.intersections(*this);
}