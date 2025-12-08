/*
* sphere.cpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la implementación de la clase Sphere,
* que representa una esfera en el espacio 3D y sus intersecciones con rayos.
*/

#include "sphere.hpp"
#include "../ray/ray.hpp"
#include <stdexcept>
#include <cmath>
#include <iostream>

using namespace std;

Sphere::Sphere(const Point& center_, double radius_, const Color& emission_, 
               const Color& kd_, const Color& ks_, const Color& kt_, double ior_)
    : center(center_), radius(radius_), emission(emission_), kd(kd_), ks(ks_), kt(kt_), ior(ior_)
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
    
    if (radius <= 0) {
        throw invalid_argument("El radio de la esfera debe ser positivo.");
    }

    if (ior < 0.0) {
        throw invalid_argument("El índice de refracción debe ser positivo.");
    }
}

/**
* Calcula la normal en un punto dado de la superficie de la esfera.
* @param p Punto en la superficie de la esfera.
* @return Dirección normal en el punto p.
*/
Direction Sphere::calculateNormalAtPoint(const Point& p) const {
    return (p - center).normalized();
}

vector<Point> Sphere::intersections(const Ray& ray) const {
    vector<Point> intersectionPoints;
    Direction oc = ray.o - center;
    double a = ray.d.dot(ray.d);
    double b = 2.0 * oc.dot(ray.d);
    double c = oc.dot(oc) - radius * radius;
    double discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return intersectionPoints; // Si no hay intersección, devolver vector vacío
    } else {
        double t1 = (-b - sqrt(discriminant)) / (2.0 * a);
        double t2 = (-b + sqrt(discriminant)) / (2.0 * a);
        
        // Solo añadir intersecciones con t positivo (delante del origen del rayo)
        if (t1 > 1e-6) {  // Usar un pequeño epsilon para evitar auto-intersección
            intersectionPoints.push_back(ray.o + ray.d * t1);
        }
        if (t2 > 1e-6 && abs(t2 - t1) > 1e-6) {  // Evitar puntos duplicados
            intersectionPoints.push_back(ray.o + ray.d * t2);
        }
        
        // Ordenar intersecciones por distancia desde el origen del rayo (la más cercana primero)
        if (intersectionPoints.size() == 2) {
            double dist1 = (intersectionPoints[0] - ray.o).norm();
            double dist2 = (intersectionPoints[1] - ray.o).norm();
            if (dist1 > dist2) {
                swap(intersectionPoints[0], intersectionPoints[1]);
            }
        }
        
        return intersectionPoints;
    }
}

bool Sphere::inSurface(const Point& p) const {
    double distSquared = (p - center).dot(p - center);
    return fabs(distSquared - radius * radius) < 1e-6; // Usar un umbral pequeño para la comparación
}

void Sphere::print() const {
    cout << "Sphere:\n  center=" << center << ", radius=" << radius << endl;
}