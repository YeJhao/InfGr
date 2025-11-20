/*
* ray.hpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la definición de la clase Ray,
* que representa un rayo en la escena.
*/

#ifndef RAY_HPP
#define RAY_HPP

#include "../geometry/geometry.hpp"
#include <vector>

// Declaración adelantada
class GeometricShape;

class Ray {
    public:
        Point o; // Origen del rayo
        Direction d; // Dirección del rayo

        Ray(const Point& origin_, const Direction& direction_);

        // Método genérico que funciona con cualquier forma geométrica, 
        // la cual DEBE tener el método intersections implementado
        std::vector<Point> intersections(const GeometricShape& shape) const;
};

#endif // RAY_HPP