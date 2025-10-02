#ifndef PLANE_HPP
#define PLANE_HPP

#include "geometry.hpp"

class Plane {
    public:
        Direction normal;
        Point origin;
        
        Plane(const Direction& normal_, const Point& point_);
};

#endif // PLANE_HPP