#include <iostream>
#include <vector>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include "ray.hpp"
#include "geometry/geometric_shape.hpp"
#include "geometry/sphere.hpp"
#include "geometry/plane.hpp"

using namespace std;

// Ray implementations
Ray::Ray(const Point& origin_, const Direction& direction_) : o(origin_), d(direction_) {}

// New generic method that works with any geometric shape
std::vector<Point> Ray::intersections(const GeometricShape& shape) const {
    return shape.intersections(*this);
}

int main() {
    cout << "=== SISTEMA DE INTERSECCIÓN CON FORMAS GEOMÉTRICAS ===" << endl;
    cout << "Configuración del rayo:" << endl;
    
    Direction rayDirection;
    double rDx, rDy, rDz;

    cout << "Dirección del rayo (dx, dy, dz): ";
    cin >> rDx >> rDy >> rDz;
    rayDirection = Direction(rDx, rDy, rDz);

    Point rayOrigin = Point(0, 0, 0);
    Ray ray(rayOrigin, rayDirection.normalized());
    
    // Mapa para almacenar las formas geométricas con nombres únicos
    map<string, unique_ptr<GeometricShape>> shapes;
    
    int opcion;
    do {
        cout << "\n=== MENÚ PRINCIPAL ===" << endl;
        cout << "1. Agregar Esfera" << endl;
        cout << "2. Agregar Plano" << endl;
        cout << "3. Listar Formas Creadas" << endl;
        cout << "4. Investigar Intersecciones con Todas las Formas" << endl;
        cout << "0. Salir" << endl;
        cout << "Selecciona una opción: ";
        cin >> opcion;
        
        switch(opcion) {
            case 1: {
                string nombre;
                cout << "\nNombre para la esfera: ";
                cin >> nombre;
                
                if (shapes.find(nombre) != shapes.end()) {
                    cout << "Error: Ya existe una forma con el nombre '" << nombre << "'" << endl;
                    break;
                }
                
                Point sphereCenter;
                double spX, spY, spZ, sphereRadius;
                
                cout << "Centro de la esfera (x, y, z): ";
                cin >> spX >> spY >> spZ;
                sphereCenter = Point(spX, spY, spZ);
                cout << "Radio: ";
                cin >> sphereRadius;
                
                shapes[nombre] = make_unique<Sphere>(sphereCenter, sphereRadius);
                cout << "Esfera '" << nombre << "' agregada exitosamente." << endl;
                break;
            }
            
            case 2: {
                string nombre;
                cout << "\nNombre para el plano: ";
                cin >> nombre;
                
                if (shapes.find(nombre) != shapes.end()) {
                    cout << "Error: Ya existe una forma con el nombre '" << nombre << "'" << endl;
                    break;
                }
                
                Direction planeNormal;
                double planeNormalX, planeNormalY, planeNormalZ;
                Point planePoint;
                double planePointX, planePointY, planePointZ;
                
                cout << "Normal del plano (dx, dy, dz): ";
                cin >> planeNormalX >> planeNormalY >> planeNormalZ;
                planeNormal = Direction(planeNormalX, planeNormalY, planeNormalZ);
                cout << "Punto en el plano (x, y, z): ";
                cin >> planePointX >> planePointY >> planePointZ;
                planePoint = Point(planePointX, planePointY, planePointZ);
                
                shapes[nombre] = make_unique<Plane>(planeNormal.normalized(), planePoint);
                cout << "Plano '" << nombre << "' agregado exitosamente." << endl;
                break;
            }
            
            case 3: {
                cout << "\n=== FORMAS GEOMÉTRICAS CREADAS ===" << endl;
                if (shapes.empty()) {
                    cout << "No hay formas geométricas creadas." << endl;
                } else {
                    int contador = 1;
                    for (const auto& pair : shapes) {
                        cout << contador << ". Nombre: '" << pair.first << "' - ";
                        pair.second->print();
                        contador++;
                    }
                }
                break;
            }
            
            case 4: {
                cout << "\n=== INVESTIGACIÓN DE INTERSECCIONES ===" << endl;
                if (shapes.empty()) {
                    cout << "No hay formas geométricas para investigar." << endl;
                    break;
                }
                
                cout << "Rayo: origen=" << rayOrigin << ", dirección=" << rayDirection << endl;
                cout << "\nResultados de intersecciones:" << endl;
                
                bool hayIntersecciones = false;
                for (const auto& pair : shapes) {
                    vector<Point> intersections = ray.intersections(*pair.second);
                    
                    cout << "\n• Forma: '" << pair.first << "'" << endl;
                    cout << "  Tipo: ";
                    pair.second->print();
                    
                    if (intersections.empty()) {
                        cout << "  Intersecciones: Ninguna" << endl;
                    } else {
                        cout << "  Intersecciones encontradas: " << intersections.size() << endl;
                        for (size_t i = 0; i < intersections.size(); ++i) {
                            cout << "    " << (i+1) << ". " << intersections[i] << endl;
                        }
                        hayIntersecciones = true;
                    }
                }
                
                if (!hayIntersecciones) {
                    cout << "\nEl rayo no intersecta con ninguna de las formas geométricas." << endl;
                } else {
                    cout << "\n¡Se encontraron intersecciones!" << endl;
                }
                break;
            }
            
            case 0:
                cout << "Saliendo del programa..." << endl;
                break;
                
            default:
                cout << "Opción no válida. Por favor, selecciona una opción del menú." << endl;
        }
        
    } while (opcion != 0);

    return 0;
}