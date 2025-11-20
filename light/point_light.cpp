/*
* point_light.cpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la implementación de la clase PointLight,
* que representa una luz puntual en la escena.
*/

#include "point_light.hpp"

PointLight::PointLight(const Point& position_, const Color& intensity_)
    : position(position_), intensity(intensity_) {}
