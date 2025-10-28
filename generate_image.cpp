#include "ray.hpp"
#include "geometry/geometric_shape.hpp"
#include "geometry/sphere.hpp"
#include "geometry/plane.hpp"
#include "geometry/triangle.hpp"
#include "imaging/imaging.hpp"
#include "light/point_light.hpp"
#include "geometry/color.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <limits>
#include <random>

using namespace std;

/*
kd, ks, kt SON COLORES! NO SON DOUBLES
FUNCIONES PARA SUMAR COLORES, MULTIPLICAR POR ESCALAR, ETC
MODULAR UN POCO TODO, PARA NO TENER FORS DENTRO DE FORS
*/

/*
 * Pre:  Text
 * Post: Text
 */
void checkIntersections(vector<unique_ptr<GeometricShape>> shapes, Ray ray, Camera camera) {
    // Variables para encontrar la intersección más cercana
    double minDistance = numeric_limits<double>::max();
    GeometricShape* closestShape = nullptr;  
    Point closestIntersection = Point(0,0,0);

    for (const auto& shape : shapes) {
        vector<Point> intersections = ray.intersections(*shape);
        
        // Para cada intersección, calcular distancia y quedarse con la más cercana
        for (const Point& intersection : intersections) {
            Direction toIntersection = intersection - camera.origin;
            double distance = toIntersection.norm();
            
            if (distance < minDistance) {
                minDistance = distance;
                closestShape = shape.get();
                closestIntersection = intersection;
            }
        }
    }
}

// Pre: 
// Post:    Adds a sphere to the shapes vector based on user input
void createSphere(vector<unique_ptr<GeometricShape>>& shapes) {
    Point center; 
    double cx, cy, cz;
    double radius;
    double r, g, b;
    double k_r, k_g, k_b;

    cout << "Centro de la esfera (x, y, z): ";
    cin >> cx >> cy >> cz;
    center = Point(cx, cy, cz);
    cout << "Radio de la esfera: ";
    cin >> radius;
    cout << "Emisión (R, G, B): ";
    cin >> r >> g >> b;
    Color rgb(r, g, b);
    cout << "Coeficiente kd (R, G, B): ";
    cin >> k_r >> k_g >> k_b;
    Color kd(k_r, k_g, k_b);
    cout << "Coeficiente ks (R, G, B): ";
    cin >> k_r >> k_g >> k_b;
    Color ks(k_r, k_g, k_b);
    cout << "Coeficiente kt (R, G, B): ";
    cin >> k_r >> k_g >> k_b;
    Color kt(k_r, k_g, k_b);

    shapes.push_back(make_unique<Sphere>(center, radius, rgb, kd, ks, kt));
    cout << "Esfera agregada exitosamente." << endl;
}

// Pre:
// Post:    Adds a plane to the shapes vector based on user input
void createPlane(vector<unique_ptr<GeometricShape>>& shapes) {
    Direction planeNormal;
    double planeNormalX, planeNormalY, planeNormalZ;
    double planeDistance;
    double r, g, b;
    double k_r, k_g, k_b;

    cout << "Normal del plano (dx, dy, dz): ";
    cin >> planeNormalX >> planeNormalY >> planeNormalZ;
    planeNormal = Direction(planeNormalX, planeNormalY, planeNormalZ);
    cout << "Distancia desde el origen al plano: ";
    cin >> planeDistance;
    cout << "Emision (R, G, B): ";
    cin >> r >> g >> b;
    Color rgb(r, g, b);
    cout << "Coeficiente kd (R, G, B): ";
    cin >> k_r >> k_g >> k_b;
    Color kd(k_r, k_g, k_b);
    cout << "Coeficiente ks (R, G, B): ";
    cin >> k_r >> k_g >> k_b;
    Color ks(k_r, k_g, k_b);
    cout << "Coeficiente kt (R, G, B): ";
    cin >> k_r >> k_g >> k_b;
    Color kt(k_r, k_g, k_b);

    shapes.push_back(make_unique<Plane>(planeNormal.normalized(), planeDistance, rgb, kd, ks, kt));
    cout << "Plano agregado exitosamente." << endl;
}

// Pre:
// Post:    Adds a triangle to the shapes vector based on user input
void createTriangle(vector<unique_ptr<GeometricShape>>& shapes) {
    double p1x, p1y, p1z, p2x, p2y, p2z, p3x, p3y, p3z;
    double r, g, b;
    double k_r, k_g, k_b;

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
    Color rgb(r, g, b);
    cout << "Coeficiente kd (R, G, B): ";
    cin >> k_r >> k_g >> k_b;
    Color kd(k_r, k_g, k_b);
    cout << "Coeficiente ks (R, G, B): ";
    cin >> k_r >> k_g >> k_b;
    Color ks(k_r, k_g, k_b);
    cout << "Coeficiente kt (R, G, B): ";
    cin >> k_r >> k_g >> k_b;
    Color kt(k_r, k_g, k_b);

    shapes.push_back(make_unique<Triangle>(p1, p2, p3, rgb, kd, ks, kt));
    cout << "Triángulo agregado exitosamente." << endl;
}

int main() {
    cout << "Tamaño de la imagen (anchura altura):" << endl;
    int pixelWidth;
    cin >> pixelWidth;
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
                createSphere(shapes);
                break;
            }
            
            case 2: {
                createPlane(shapes);
                break;
            }
            
            case 3: {
                createTriangle(shapes);
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

            // Generar imagen con cámara configurable
            case 5: {
                if (shapes.empty()) {
                    cout << "No hay formas geométricas creadas. Agregue algunas primero." << endl;
                    break;
                }
                
                cout << "\n=== CONFIGURACIÓN DE CÁMARA ===" << endl;
                
                // Configurar cámara
                Point cameraOrigin;
                Direction l, u, f; // vectores base de la cámara
                double ox, oy, oz;
                double lx, ly, lz, ux, uy, uz, fx, fy, fz;
                
                cout << "Origen de la cámara (x, y, z): ";
                cin >> ox >> oy >> oz;
                cameraOrigin = Point(ox, oy, oz);
                
                cout << "Vector L (left/izquierda) (x, y, z): ";
                cin >> lx >> ly >> lz;
                l = Direction(lx, ly, lz);
                
                cout << "Vector U (up/arriba) (x, y, z): ";
                cin >> ux >> uy >> uz;
                u = Direction(ux, uy, uz);

                cout << "Vector F (front/adelante) (x, y, z): ";
                cin >> fx >> fy >> fz;
                f = Direction(fx, fy, fz);

                // Crear la cámara
                Camera camera(cameraOrigin, l, u, f);

                cout << "\n=== CONFIGURACIÓN DE LA LUZ ===" << endl;
                double lightX, lightY, lightZ;
                double intR, intG, intB;
                cout << "Posición de la luz (x, y, z): ";
                cin >> lightX >> lightY >> lightZ;
                Point lightPosition(lightX, lightY, lightZ);
                cout << "Intensidad de la luz (R, G, B): ";
                cin >> intR >> intG >> intB;
                Color lightIntensity(intR, intG, intB);

                // Crear la fuente de luz (luz puntual)
                PointLight light(lightPosition, lightIntensity);

                cout << "\n=== GENERANDO IMAGEN ===" << endl;
                
                // Crear la imagen
                Image image(pixelWidth, pixelHeight);
                
                // Generador de números aleatorios
                random_device rd;
                mt19937 gen(rd());
                uniform_real_distribution<double> dis(0.0, 1.0);
                
                const int raysPerPixel = 4; // Número de rayos por píxel para anti-aliasing
                
                Color geometryKd(0, 0, 0);
                Color geometryKs(0, 0, 0);
                Color geometryKt(0, 0, 0);

                // Para cada píxel
                for (int i = 0; i < pixelHeight; ++i) {
                    for (int j = 0; j < pixelWidth; ++j) {
                        // Calcular los límites del píxel en coordenadas del mundo
                        double pixelSizeX = 2.0 / pixelWidth;   // Tamaño de un píxel en X
                        double pixelSizeY = 2.0 / pixelHeight; // Tamaño de un píxel en Y
                        
                        // Esquina inferior-izquierda del píxel en coordenadas de cámara
                        double pixelMaxX = 1.0 - j * pixelSizeX;
                        double pixelMinX = pixelMaxX - pixelSizeX;
                        double pixelMaxY = 1.0 - i * pixelSizeY;
                        double pixelMinY = pixelMaxY - pixelSizeY;
                        
                        // Acumuladores para el color
                        //double totalR = 0.0, totalG = 0.0, totalB = 0.0;
                        Color total(0, 0, 0);

                        // Lanzar múltiples rayos por píxel (Monte Carlo)
                        for (int ray_sample = 0; ray_sample < raysPerPixel; ++ray_sample) {
                            // Generar coordenadas aleatorias dentro del píxel en coordenadas de cámara
                            double x = pixelMinX + dis(gen) * pixelSizeX;
                            double y = pixelMinY + dis(gen) * pixelSizeY;
                            double z = 1; // Plano de imagen en z=1 en coordenadas de cámara
                            
                            // Convertir punto del plano de imagen de coordenadas de cámara a mundo
                            Point cameraPixelPoint(x, y, z);
                            Point cameraLocalOrigin = Point(0,0,0);
                            Direction cameraDirection = cameraPixelPoint - cameraLocalOrigin;
                            Direction rayDirection = camera.cameraToWorld(cameraDirection);
                            
                            // Crear el rayo desde el origen de la cámara hacia el punto del píxel
                            Ray ray(camera.origin, rayDirection.normalized());
                            
                            // Variables para encontrar la intersección más cercana
                            double minDistance = numeric_limits<double>::max();
                            GeometricShape* closestShape = nullptr;  
                            Point closestIntersection = Point(0,0,0);                    
                            
                            // Comprobar intersección con todas las formas
                            // TODO: Seguir con el check
                            //checkIntersections(shapes, ray, camera);
                            for (const auto& shape : shapes) {
                                vector<Point> intersections = ray.intersections(*shape);
                                
                                // Para cada intersección, calcular distancia y quedarse con la más cercana
                                for (const Point& intersection : intersections) {
                                    Direction toIntersection = intersection - camera.origin;
                                    double distance = toIntersection.norm();
                                    
                                    if (distance < minDistance) {
                                        minDistance = distance;
                                        closestShape = shape.get();
                                        closestIntersection = intersection;
                                    }
                                }
                            }

                            Color geometryColor(0, 0, 0); // Negro por defecto
                            
                            Direction geometryNormal(0, 0, 0);

                            if (closestShape) {
                                if (auto sphere = dynamic_cast<const Sphere*>(closestShape)) {
                                    geometryColor = Color(sphere->emission.r, sphere->emission.g, sphere->emission.b);
                                    geometryKd = Color(sphere->kd.r, sphere->kd.g, sphere->kd.b);
                                    geometryKs = Color(sphere->ks.r, sphere->ks.g, sphere->ks.b);
                                    geometryKt = Color(sphere->kt.r, sphere->kt.g, sphere->kt.b);
                                    geometryNormal = sphere->calculateNormalAtPoint(closestIntersection);
                                } else if (auto plane = dynamic_cast<const Plane*>(closestShape)) {
                                    geometryColor = Color(plane->emission.r, plane->emission.g, plane->emission.b);
                                    geometryKd = Color(plane->kd.r, plane->kd.g, plane->kd.b);
                                    geometryKs = Color(plane->ks.r, plane->ks.g, plane->ks.b);
                                    geometryKt = Color(plane->kt.r, plane->kt.g, plane->kt.b);
                                    geometryNormal = plane->normal;
                                } else if (auto triangle = dynamic_cast<const Triangle*>(closestShape)) {
                                    geometryColor = Color(triangle->emission.r, triangle->emission.g, triangle->emission.b);
                                    geometryKd = Color(triangle->kd.r, triangle->kd.g, triangle->kd.b);
                                    geometryKs = Color(triangle->ks.r, triangle->ks.g, triangle->ks.b);
                                    geometryKt = Color(triangle->kt.r, triangle->kt.g, triangle->kt.b);
                                    geometryNormal = triangle->normal;
                                }
                            }

                            // Calcular la iluminación del rayo
                            // Solo difuso
                            Direction wi = light.position - closestIntersection; // Intersección a la luz
                            Direction wo = camera.origin - closestIntersection; // Intersección a la cámara

                            // color.hpp METER OPERACIONES SUMA, PRODUCTO * ESCALAR, ETC  de color
                            double moduloWi = wi.norm();
                            Color incomingLight = light.intensity / (moduloWi * moduloWi);
                            //double incomingLightR = light.intensity.r / (moduloWi*moduloWi);
                            //double incomingLightG = light.intensity.g / (moduloWi*moduloWi);
                            //double incomingLightB = light.intensity.b / (moduloWi*moduloWi);

                            Color fr = geometryKd / M_PI;

                            Direction wi2 = wi/moduloWi;
                            double coseno = geometryNormal.dot(wi2);

                            // Acumular el color de este rayo
                            //totalR += incomingLightR * fr.r * coseno;
                            //totalG += incomingLightG * fr.g * coseno;
                            //totalB += incomingLightB * fr.b * coseno;
                            total = total + (incomingLight * fr * coseno);
                        }
                        
                        // Promediar los colores de todos los rayos
                        //double avgR = totalR / raysPerPixel;
                        //double avgG = totalG / raysPerPixel;
                        //double avgB = totalB / raysPerPixel;
                        Color avg = total / static_cast<double>(raysPerPixel);

                        // Asignar el color promedio al píxel
                        image.imagen[i][j] = PixelRGB(avg.r, avg.g, avg.b);
                    }
                    
                    // Mostrar progreso cada 10% de las filas
                    if ((i + 1) % (pixelHeight / 10) == 0 || i == pixelHeight - 1) {
                        int progress = ((i + 1) * 100) / pixelHeight;
                        cout << "Progreso: " << progress << "% (" << (i + 1) << "/" << pixelHeight << " filas)" << endl;
                    }
                }
                
                // Guardar la imagen
                cout << "\nNombre de la imagen (sin .png): ";
                string nameInput;
                cin >> nameInput;
                string filename = nameInput + ".png";
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