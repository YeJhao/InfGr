#ifndef POINT_LIGHT_HPP
#define POINT_LIGHT_HPP

#include "geometry/geometry.hpp"
#include "geometry/color.hpp"

class PointLight {
    public:
        Point position;
        Color intensity; // Color e intensidad de la luz

        PointLight(const Point& position_, const Color& intensity_);
};

#endif // POINT_LIGHT_HPP