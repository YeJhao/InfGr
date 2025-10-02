#include "plane.hpp"

using namespace std;

Plane::Plane(const Direction& normal_, const Point& point_) : normal(normal_), origin(point_) {}