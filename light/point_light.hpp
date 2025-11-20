/*
* point_light.cpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la definición de la clase PointLight,
* que representa una luz puntual en la escena.
*/

#ifndef POINT_LIGHT_HPP
#define POINT_LIGHT_HPP

#include "geometry/geometry.hpp"
#include "geometry/color.hpp"

class PointLight {
    public:
        Point position;  // Posición de la luz puntual
        Color intensity; // Color e intensidad de la luz

        PointLight(const Point& position_, const Color& intensity_);
};

#endif // POINT_LIGHT_HPP