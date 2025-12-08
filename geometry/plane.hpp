/*
* plane.hpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la definición de la clase Plane,
* que representa un plano en el espacio 3D y sus intersecciones con rayos.
*/

#ifndef PLANE_HPP
#define PLANE_HPP

#include "geometry.hpp"
#include "geometric_shape.hpp"
#include "color.hpp"

using namespace std;

class Plane : public GeometricShape {
    public:
        Direction normal; // Vector normal del plano
        double distance; // Distancia con signo desde el origen al plano
        Color emission;  // Emisión de la geometría
        Color kd, ks, kt; // Coeficientes de reflexión difusa, especular y transmisiva
        double ior = 1.0; // Índice de refracción

        Plane(const Direction& normal_, double distance_, const Color& emission_, 
              const Color& kd_, const Color& ks_, const Color& kt_, double ior_ = 1.0);

        // Sobrescribe el método virtual puro
        vector<Point> intersections(const Ray& ray) const override;
        bool inSurface(const Point& p) const override;
        
        // Sobrescribe el método de impresión
        void print() const override;
};

#endif // PLANE_HPP