/*
* sphere.hpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la definición de la clase Sphere,
* que representa una esfera en el espacio 3D y sus intersecciones con rayos.
*/

#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "geometry.hpp"
#include "geometric_shape.hpp"
#include "color.hpp"

class Sphere : public GeometricShape {
    public:
        Point center; // Centro de la esfera
        double radius; // Radio de la esfera
        Color emission; // Emisión de la esfera
        Color kd, ks, kt; // Coeficientes de reflexión difusa, especular y transmisiva
        double ior = 1.0; // Índice de refracción

        Sphere(const Point& center_, double radius_, const Color& emission_, 
               const Color& kd_, const Color& ks_, const Color& kt_, double ior_ = 1.0);

        Direction calculateNormalAtPoint(const Point& p) const;

        // Sobrescribe el método virtual puro
        std::vector<Point> intersections(const Ray& ray) const override;
        
        // Sobrescribe el método de impresión
        void print() const override;
};

#endif // SPHERE_HPP