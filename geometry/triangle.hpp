/*
* triangle.hpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la definición de la clase Triangle,
* que representa un triángulo en el espacio 3D y sus intersecciones con rayos.
*/

#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include "geometry.hpp"
#include "geometric_shape.hpp"
#include "color.hpp"

using namespace std;

class Triangle : public GeometricShape {
    public:
        Point v0, v1, v2; // Los tres vértices del triángulo
        Direction normal; // Vector normal (calculado a partir de los vértices)
        Color emission;  // Emisión del triángulo
        Color kd, ks, kt; // Coeficientes para el triángulo
        double ior = 1.0; // Índice de refracción

        Triangle(const Point& vertex0, const Point& vertex1, const Point& vertex2, const Color& emission_, 
                 const Color& kd_, const Color& ks_, const Color& kt_, double ior_ = 1.0);

        // Sobrescribe el método virtual puro
        vector<Point> intersections(const Ray& ray) const override;
        bool inSurface(const Point& p) const;
        
        // Sobrescribe el método de impresión
        void print() const override;
        
    private:
        // Método auxiliar para calcular la normal a partir de los vértices
        void computeNormal();
        
        
};

#endif // TRIANGLE_HPP