/*
* geometric_shape.hpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la definición de la clase GeometricShape,
* que representa formas geométricas y sus intersecciones con rayos.
*/

#ifndef GEOMETRIC_SHAPE_HPP
#define GEOMETRIC_SHAPE_HPP

#include "geometry.hpp" 
#include <vector>
#include "../ray/ray.hpp"

using namespace std;

class GeometricShape {
public:
    virtual ~GeometricShape() = default;
    
    // Función para calcular intersecciones con un rayo, debe estar implementada por las subclases
    virtual vector<Point> intersections(const Ray& ray) const = 0;

    virtual bool inSurface(const Point& p) const = 0;
    
    // Opcional: función para imprimir información sobre la forma geométrica
    virtual void print() const = 0;
};

#endif // GEOMETRIC_SHAPE_HPP