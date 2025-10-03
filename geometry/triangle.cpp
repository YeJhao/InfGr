#include "triangle.hpp"
#include "../ray.hpp"
#include <iostream>
#include <cmath>

using namespace std;

Triangle::Triangle(const Point& vertex0, const Point& vertex1, const Point& vertex2) 
    : v0(vertex0), v1(vertex1), v2(vertex2) {
    computeNormal();
}

void Triangle::computeNormal() {
    Direction edge1 = v1 - v0;
    Direction edge2 = v2 - v0;
    normal = edge1.cross(edge2).normalized();
}

std::vector<Point> Triangle::intersections(const Ray& ray) const {
    vector<Point> intersectionPoints;
    
    // First, find intersection with the plane containing the triangle
    double denom = ray.d.dot(normal);
    if (fabs(denom) < 1e-6) {
        return intersectionPoints; // Ray is parallel to triangle plane
    }
    
    double t = (v0 - ray.o).dot(normal) / denom;
    if (t < 0) {
        return intersectionPoints; // Intersection is behind ray origin
    }
    
    Point intersectionPoint = ray.o + ray.d * t;
    
    // Check if the intersection point is inside the triangle using barycentric coordinates
    if (isPointInside(intersectionPoint)) {
        intersectionPoints.push_back(intersectionPoint);
    }
    
    return intersectionPoints;
}

bool Triangle::isPointInside(const Point& p) const {
    // Using barycentric coordinates method
    Direction v0v1 = v1 - v0;
    Direction v0v2 = v2 - v0;
    Direction v0p = p - v0;
    
    double dot00 = v0v2.dot(v0v2);
    double dot01 = v0v2.dot(v0v1);
    double dot02 = v0v2.dot(v0p);
    double dot11 = v0v1.dot(v0v1);
    double dot12 = v0v1.dot(v0p);
    
    double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
    double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    
    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

void Triangle::print() const {
    cout << "Triangle: v0=" << v0 << ", v1=" << v1 << ", v2=" << v2 
         << ", normal=" << normal << endl;
}