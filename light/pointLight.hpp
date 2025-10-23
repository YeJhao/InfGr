#ifndef POINTLIGHT_HPP
#define POINTLIGHT_HPP

#include "geometry/geometry.hpp"
#include "color.hpp"

class PointLight {
    public:
        Point position;
        Color intensity; // Color e intensidad de la luz

        PointLight(const Point& position_, const Color& intensity_);
};

#endif // POINTLIGHT_HPP