#include <iostream>
#include <vector>
#include <cmath>
#include "ray.hpp"

using namespace std;

// Ray implementations
Ray::Ray(const Point& origin_, const Direction& direction_) : o(origin_), d(direction_) {}

vector<Point> Ray::sphereIntersections(const Sphere& sphere) {
    vector<Point> intersections;
    Direction oc = o - sphere.center;
    double a = d.dot(d);
    double b = 2.0 * oc.dot(d);
    double c = oc.dot(oc) - sphere.radius * sphere.radius;
    double discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return intersections; // No intersection, empty vector
    } else {
        double t1 = (-b - sqrt(discriminant)) / (2.0 * a);
        double t2 = (-b + sqrt(discriminant)) / (2.0 * a);
        if (t1 >= 0) {
            intersections.push_back(o + d * t1);
        }
        if (t2 >= 0 && t2 != t1) {
            intersections.push_back(o + d * t2);
        }
        if (t1 > t2) {
            intersections.front() = o + d * t2;
            intersections.back() = o + d * t1;
        }
        return intersections;
    }
}

Point Ray::planeIntersection(const Plane& plane) {
    double denom = d.dot(plane.normal);
    
    if (fabs(denom) > 1e-6) { // Ensure the ray is not parallel to the plane
        double t = (plane.origin - o).dot(plane.normal) / denom;
        if (t >= 0) {
            return o + t * d; // Intersection point
        }
    }
    // Return a point far away to indicate no intersection (better than nullptr for Point)
    return Point(INFINITY, INFINITY, INFINITY);
}

int main() {
    cout << "Especifica un rayo (origen y dirección): " << endl;
    Point rayOrigin;
    double rOx, rOy, rOz;
    Direction rayDirection;
    double rDx, rDy, rDz;

    cout << "Origen (x, y, z): ";
    cin >> rOx >> rOy >> rOz;
    rayOrigin = Point(rOx, rOy, rOz);
    cout << "Dirección (dx, dy, dz): ";
    cin >> rDx >> rDy >> rDz;
    rayDirection = Direction(rDx, rDy, rDz);

    Ray ray(rayOrigin, rayDirection.normalized());

    
    cout << "Especifica una esfera (centro y radio): " << endl;
    Point sphereCenter;
    double spX, spY, spZ;
    double sphereRadius;
    
    cout << "Centro (x, y, z): ";
    cin >> spX >> spY >> spZ;
    sphereCenter = Point(spX, spY, spZ);
    cout << "Radio: ";
    cin >> sphereRadius;

    Sphere sphere(sphereCenter, sphereRadius);

    vector<Point> intersections = ray.sphereIntersections(sphere);
    if (intersections.empty()) {
        cout << "No hay intersección con la esfera." << endl;
    } else {
        cout << "Intersecciones con la esfera:" << endl;
        cout << "Número de intersecciones: " << intersections.size() << endl;
        cout << "Intersección más importante: " << intersections.front() << endl;
    }

    cout << "Especifica un plano (normal y un punto contenido en él): " << endl;
    Direction planeNormal;
    double planeNormalX, planeNormalY, planeNormalZ;
    Point planePoint;
    double planePointX, planePointY, planePointZ;
    cout << "Normal (dx, dy, dz): ";
    cin >> planeNormalX >> planeNormalY >> planeNormalZ;
    planeNormal = Direction(planeNormalX, planeNormalY, planeNormalZ);
    cout << "Punto (x, y, z): ";
    cin >> planePointX >> planePointY >> planePointZ;
    planePoint = Point(planePointX, planePointY, planePointZ);

    Plane plane(planeNormal.normalized(), planePoint);

    Point intersection = ray.planeIntersection(plane);
    // Check if intersection is valid (not at infinity)
    if (!isinf(intersection.x()) && !isinf(intersection.y()) && !isinf(intersection.z())) {
        cout << "Intersección con el plano: " << intersection << endl;
    } else {
        cout << "No hay intersección con el plano." << endl;
    }

    return 0;
}