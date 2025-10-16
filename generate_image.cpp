#include "ray.hpp"
#include "geometry/geometric_shape.hpp"
#include "geometry/sphere.hpp"
#include "geometry/plane.hpp"
#include "geometry/triangle.hpp"
#include "imaging/imaging.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <limits>
#include <random>

using namespace std;

int main() {
    cout << "Qué anchura tiene la imagen?" << endl;
    int width;
    cin >> width;
    cout << "Qué altura tiene la imagen?" << endl;
    int height;
    cin >> height;

    cout << "Cuántos píxeles quieres generar en anchura?" << endl;
    int pixelWidth;
    cin >> pixelWidth;
    cout << "Cuántos píxeles quieres generar en altura?" << endl;
    int pixelHeight;
    cin >> pixelHeight;

    int numPixels = pixelWidth * pixelHeight;

    vector<unique_ptr<GeometricShape>> shapes;

    int opcion;
    do {
        cout << "\n=== MENÚ PRINCIPAL ===" << endl;
        cout << "1. Agregar Esfera" << endl;
        cout << "2. Agregar Plano" << endl;
        cout << "3. Agregar Triángulo" << endl;
        cout << "4. Listar Formas Creadas" << endl;
        cout << "5. Generar imagen" << endl;
        cout << "0. Salir" << endl;
        cout << "Selecciona una opción: ";
        cin >> opcion;

        switch(opcion) {
            case 1: {
                Point center; 
                double cx, cy, cz;
                double radius;
                double r, g, b;
                cout << "Centro de la esfera (x, y, z): ";
                cin >> cx >> cy >> cz;
                center = Point(cx, cy, cz);
                cout << "Radio de la esfera: ";
                cin >> radius;
                cout << "Emisión (R, G, B): ";
                cin >> r >> g >> b;
                shapes.push_back(make_unique<Sphere>(center, radius, Color(r, g, b)));
                cout << "Esfera agregada exitosamente." << endl;
                break;
            }
            
            case 2: {
                Direction planeNormal;
                double planeNormalX, planeNormalY, planeNormalZ;
                Point planePoint;
                double planePointX, planePointY, planePointZ;
                double r, g, b;

                cout << "Normal del plano (dx, dy, dz): ";
                cin >> planeNormalX >> planeNormalY >> planeNormalZ;
                planeNormal = Direction(planeNormalX, planeNormalY, planeNormalZ);
                cout << "Punto en el plano (x, y, z): ";
                cin >> planePointX >> planePointY >> planePointZ;
                planePoint = Point(planePointX, planePointY, planePointZ);
                cout << "Emision (R, G, B): ";
                cin >> r >> g >> b;

                shapes.push_back(make_unique<Plane>(planeNormal.normalized(), planePoint, Color(r, g, b)));
                cout << "Plano agregado exitosamente." << endl;
                break;
            }
            
            case 3: {
                double p1x, p1y, p1z, p2x, p2y, p2z, p3x, p3y, p3z;
                double r, g, b;
                cout << "Vértices del triángulo (p1 x y z): ";
                cin >> p1x >> p1y >> p1z;
                cout << "Vértices del triángulo (p2 x y z): ";
                cin >> p2x >> p2y >> p2z;
                cout << "Vértices del triángulo (p3 x y z): ";
                cin >> p3x >> p3y >> p3z;
                Point p1(p1x, p1y, p1z);
                Point p2(p2x, p2y, p2z);
                Point p3(p3x, p3y, p3z);
                cout << "Emision (R, G, B): ";
                cin >> r >> g >> b;

                shapes.push_back(make_unique<Triangle>(p1, p2, p3, Color(r, g, b)));
                cout << "Triángulo agregado exitosamente." << endl;
                break;
            }

            case 4: {
                cout << "\n=== LISTA DE FORMAS CREADAS ===" << endl;
                if (shapes.empty()) {
                    cout << "No hay formas geométricas creadas." << endl;
                } else {
                    int contador = 1;
                    for (const auto& shape : shapes) {
                        cout << contador << ". ";
                        shape->print();
                        contador++;
                    }
                }
                break;
            }

            // De momento, la cámara está fija en el origen (0,0,0) y con coordenadas (1,1,1),
            // pero eventualmente tendremos que dar valores al origen y a la dirección de la cámara
            case 5: {
                cout << "\n=== GENERANDO IMAGEN ===" << endl;
                
                // Crear la imagen
                Image image(pixelWidth, pixelHeight);
                
                // Cámara en el origen
                Point camera(0, 0, 0);
                
                
                
                // Generador de números aleatorios
                random_device rd;
                mt19937 gen(rd());
                uniform_real_distribution<double> dis(0.0, 1.0);
                
                const int raysPerPixel = 64; // Número de rayos por píxel para anti-aliasing
                
                cout << "Generando " << numPixels << " píxeles con anti-aliasing (" 
                     << raysPerPixel << " rayos por píxel)..." << endl;

                // Para cada píxel
                for (int i = 0; i < pixelHeight; ++i) {
                    for (int j = 0; j < pixelWidth; ++j) {
                        // Calcular los límites del píxel en coordenadas del mundo
                        double pixelSizeX = 2.0 / pixelWidth;   // Tamaño de un píxel en X
                        double pixelSizeY = 2.0 / pixelHeight; // Tamaño de un píxel en Y
                        
                        // Esquina inferior-izquierda del píxel
                        double pixelMinX = -1.0 + j * pixelSizeX;
                        double pixelMaxX = pixelMinX + pixelSizeX;
                        double pixelMaxY = 1.0 - i * pixelSizeY;
                        double pixelMinY = pixelMaxY - pixelSizeY;
                        
                        // Acumuladores para el color
                        double totalR = 0.0, totalG = 0.0, totalB = 0.0;
                        
                        // Lanzar múltiples rayos por píxel (Monte Carlo)
                        for (int ray_sample = 0; ray_sample < raysPerPixel; ++ray_sample) {
                            // Generar coordenadas aleatorias dentro del píxel
                            double x = pixelMinX + dis(gen) * pixelSizeX;
                            double y = pixelMinY + dis(gen) * pixelSizeY;
                            double z = 1.0;
                            
                            // Crear el rayo desde la cámara hacia el punto aleatorio
                            Point pixelPoint(x, y, z);
                            Direction rayDirection = pixelPoint - camera;
                            Ray ray(camera, rayDirection.normalized());
                            
                            // Variables para encontrar la intersección más cercana
                            double minDistance = numeric_limits<double>::max();
                            Color rayColor(0, 0, 0); // Negro por defecto
                            
                            // Comprobar intersección con todas las formas
                            for (const auto& shape : shapes) {
                                vector<Point> intersections = ray.intersections(*shape);
                                
                                // Para cada intersección, calcular distancia y quedarse con la más cercana
                                for (const Point& intersection : intersections) {
                                    Direction toIntersection = intersection - camera;
                                    double distance = toIntersection.norm();
                                    
                                    if (distance < minDistance) {
                                        minDistance = distance;
                                        
                                        // Obtener el color de la forma
                                        // Usar dynamic_cast para obtener el color específico
                                        if (auto sphere = dynamic_cast<const Sphere*>(shape.get())) {
                                            rayColor = Color(sphere->emission.r, sphere->emission.g, sphere->emission.b);
                                        } else if (auto plane = dynamic_cast<const Plane*>(shape.get())) {
                                            rayColor = Color(plane->emission.r, plane->emission.g, plane->emission.b);
                                        } else if (auto triangle = dynamic_cast<const Triangle*>(shape.get())) {
                                            rayColor = Color(triangle->emission.r, triangle->emission.g, triangle->emission.b);
                                        }
                                    }
                                }
                            }
                            
                            // Acumular el color de este rayo
                            totalR += rayColor.r;
                            totalG += rayColor.g;
                            totalB += rayColor.b;
                        }
                        
                        // Promediar los colores de todos los rayos
                        double avgR = totalR / raysPerPixel;
                        double avgG = totalG / raysPerPixel;
                        double avgB = totalB / raysPerPixel;
                        
                        // Asignar el color promedio al píxel
                        image.imagen[i][j] = PixelRGB(avgR, avgG, avgB);
                    }
                }
                
                // Para cada píxel
                /*for (int i = 0; i < pixelHeight; ++i) {
                    for (int j = 0; j < pixelWidth; ++j) {
                        // Calcular coordenadas del píxel en el plano z=1
                        // Mapear desde índices [0, pixelWidth-1] x [0, pixelHeight-1]
                        // a coordenadas [-1, 1] x [1, -1] (y va de arriba a abajo)
                    
                        /* Anterior: 1 rayo/píxel -> en el centrro
                        double x = -1.0 + (2.0 * j) / (pixelWidth - 1);
                        double y = 1.0 - (2.0 * i) / (pixelHeight - 1);
                        double z = 1.0;
                                               
                        // Crear el rayo desde la cámara hacia el píxel
                        Point pixelPoint(x, y, z);
                        Direction rayDirection = pixelPoint - camera;
                        Ray ray(camera, rayDirection.normalized());
                        
                        // Variables para encontrar la intersección más cercana
                        double minDistance = numeric_limits<double>::max();
                        Color pixelColor(0, 0, 0); // Negro por defecto
                        
                        // Comprobar intersección con todas las formas
                        for (const auto& shape : shapes) {
                            vector<Point> intersections = ray.intersections(*shape);
                            
                            // Para cada intersección, calcular distancia y quedarse con la más cercana
                            for (const Point& intersection : intersections) {
                                Direction toIntersection = intersection - camera;
                                double distance = toIntersection.norm();
                                
                                if (distance < minDistance) {
                                    minDistance = distance;
                                    
                                    // Obtener el color de la forma
                                    // Usar dynamic_cast para obtener el color específico
                                    if (auto sphere = dynamic_cast<const Sphere*>(shape.get())) {
                                        pixelColor = Color(sphere->emission.r, sphere->emission.g, sphere->emission.b);
                                    } else if (auto plane = dynamic_cast<const Plane*>(shape.get())) {
                                        pixelColor = Color(plane->emission.r, plane->emission.g, plane->emission.b);
                                    } else if (auto triangle = dynamic_cast<const Triangle*>(shape.get())) {
                                        pixelColor = Color(triangle->emission.r, triangle->emission.g, triangle->emission.b);
                                    }
                                }
                            }
                        }
                        
                        // Asignar el color al píxel
                        image.imagen[i][j] = PixelRGB(pixelColor.r, pixelColor.g, pixelColor.b);
                    }
                }*/
                
                // Guardar la imagen
                string filename = "generated_image.png";
                cout << "Nombre de la imagen (sin .png): ";
                string nameInput;
                cin >> nameInput;
                filename = nameInput + ".png";
                cout << "Guardando imagen como: " << filename << endl;
                
                try {
                    savePNGImage(image, filename);
                    cout << "¡Imagen generada exitosamente!" << endl;
                } catch (const exception& e) {
                    cout << "Error al guardar la imagen: " << e.what() << endl;
                }
                
                break;
            }
            
            default: {
                cout << "chao" << endl;
            }
        }
    } while (opcion > 0 && opcion < 6);

    return 1;
}