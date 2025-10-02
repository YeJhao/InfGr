#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "geometry.hpp"

class Sphere {
    public: 
        Point center;
        double radius;

        Sphere(const Point& center_, double radius_);
};

#endif // SPHERE_HPP