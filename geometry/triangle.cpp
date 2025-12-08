/*
* triangle.cpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la implementación de la clase Triangle,
* que representa un triángulo en el espacio 3D y sus intersecciones con rayos.
*/

#include "triangle.hpp"
#include "../ray/ray.hpp"
#include <iostream>
#include <cmath>

using namespace std;

Triangle::Triangle(const Point& vertex0, const Point& vertex1, const Point& vertex2, const Color& emission_, 
                   const Color& kd_, const Color& ks_, const Color& kt_, double ior_)
    : v0(vertex0), v1(vertex1), v2(vertex2), emission(emission_), kd(kd_), ks(ks_), kt(kt_), ior(ior_)
{
    // Asegura que los valores sean positivos
    if (emission.r < 0 || emission.g < 0 || emission.b < 0) {
        throw invalid_argument("Los valores de emision deben ser no negativos");
    }

    if (kd.r < 0 || kd.g < 0 || kd.b < 0 || 
        ks.r < 0 || ks.g < 0 || ks.b < 0 || 
        kt.r < 0 || kt.g < 0 || kt.b < 0) {
        throw invalid_argument("Los coeficientes de color deben ser no negativos");
    }
    
    // Validar que la suma de coeficientes no exceda 1 en cada canal
    if (kd.r + ks.r + kt.r > 1.0 || 
        kd.g + ks.g + kt.g > 1.0 || 
        kd.b + ks.b + kt.b > 1.0) {
        throw invalid_argument("La suma de coeficientes (kd + ks + kt) debe ser <= 1 en cada canal RGB");
    }

    if (ior < 0.0) {
        throw invalid_argument("El índice de refracción debe ser positivo.");
    }
    
    computeNormal();
}

/**
* Calcula la normal del triángulo usando el producto cruzado de dos aristas.
*/
void Triangle::computeNormal() {
    Direction edge1 = v1 - v0;
    Direction edge2 = v2 - v0;
    normal = edge1.cross(edge2).normalized();
}

/**
* Verifica si un punto dado está dentro del triángulo usando coordenadas baricéntricas.
* @param p Punto a verificar.
* @return true si el punto está dentro del triángulo, false en caso contrario.
*/
bool Triangle::inSurface(const Point& p) const {
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

vector<Point> Triangle::intersections(const Ray& ray) const {
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
    if (inSurface(intersectionPoint)) {
        intersectionPoints.push_back(intersectionPoint);
    }
    
    return intersectionPoints;
}

void Triangle::print() const {
    cout << "Triangle: \n  v0=" << v0 << ", v1=" << v1 << ", v2=" << v2 
         << ", normal=" << normal << endl;
}