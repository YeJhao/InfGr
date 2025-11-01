#include "ray.hpp"
#include "geometry/geometric_shape.hpp"
#include "geometry/sphere.hpp"
#include "geometry/plane.hpp"
#include "geometry/triangle.hpp"
#include "geometry/bsdf_utils.hpp"
#include "path_tracing.hpp"
#include "imaging/imaging.hpp"
#include "light/point_light.hpp"
#include "geometry/color.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <limits>
#include <random>
#include <chrono>
#include <iomanip>

using namespace std;

// Configuración de Path Tracing
#define maxBounces 100          // Límite de seguridad, Russian Roulette terminará antes
#define RR_MIN_DEPTH 2         // Profundidad mínima antes de aplicar Russian Roulette
#define RR_STOP_PROB 0.04      // Probabilidad de terminar en la Russian Roulette

int current_scene = 1;
int raysPerPixel = 512;       // Rayos por píxel (SPP)

// Pre: shapes y lights ya creados
// Post: carga en shapes y lights la escena de Cornell Box con luz puntual
void cb_onePL_difuse_spheres(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights) {
    shapes.clear();
    lights.clear();

    // GEOMETRÍA
    Plane planoArriba(Direction(0, -1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoArriba));

    Plane planoAbajo(Direction(0, 1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoAbajo));

    Plane planoFondo(Direction(0, 0, -1), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoFondo));

    Plane planoIzquierda(Direction(1, 0, 0), 1, Color(0,0,0), Color(0.8, 0.2, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoIzquierda));

    Plane planoDerecha(Direction(-1, 0, 0), 1, Color(0,0,0), Color(0.2, 0.8, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoDerecha));

    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0.8, 0.6, 0.9), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));

    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0, 0, 0), Color(0.5, 0.9, 0.9), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaDerecha));


    // LUZ PUNTUAL
    PointLight light(Point(0, 0.5, 0), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light));
}

// Pre: shapes y lights ya creados
// Post: carga en shapes y lights la escena de Cornell Box con luz de área (plano superior emisor)
void cb_top_AL(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights) {
    shapes.clear();
    lights.clear();

    // GEOMETRÍA    
    Plane planoAbajo(Direction(0, 1, 0), 1, Color(0,0,0), Color(0.8,0.8,0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoAbajo));
    
    Plane planoFondo(Direction(0, 0, -1), 1, Color(0,0,0), Color(0.8,0.8,0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoFondo));
    
    Plane planoIzquierda(Direction(1, 0, 0), 1, Color(0,0,0), Color(0.8, 0.2, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoIzquierda));

    Plane planoDerecha(Direction(-1, 0, 0), 1, Color(0,0,0), Color(0.2, 0.8, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoDerecha));

    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0.8, 0.6, 0.9), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));
    
    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0, 0, 0), Color(0.5, 0.9, 0.9), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaDerecha));


    // LUZ DE ÁREA 
    Plane areaLight(Direction(0, -1, 0), 1, Color(1,1,1), Color(0,0,0), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(areaLight));
}

// Pre: shapes y lights ya creados
// Post: carga en shapes y lights la escena de Cornell Box con dos luces puntuales
void cb_twoPL_difuse_spheres(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights) {
    shapes.clear();
    lights.clear();

    // GEOMETRÍA
    Plane planoArriba(Direction(0, -1, 0), 1, Color(0,0,0), Color(0.8,0.8,0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoArriba));
    
    Plane planoAbajo(Direction(0, 1, 0), 1, Color(0,0,0), Color(0.8,0.8,0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoAbajo));
    
    Plane planoFondo(Direction(0, 0, -1), 1, Color(0,0,0), Color(0.8,0.8,0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoFondo));
    
    Plane planoIzquierda(Direction(1, 0, 0), 1, Color(0,0,0), Color(0.8, 0.2, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoIzquierda));

    Plane planoDerecha(Direction(-1, 0, 0), 1, Color(0,0,0), Color(0.2, 0.8, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoDerecha));

    // Esferas difusas
    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0.8, 0.6, 0.9), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));
    
    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0, 0, 0), Color(0.5, 0.9, 0.9), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaDerecha));
    

    // DOS LUCES PUNTUALES
    PointLight light1(Point(1, 0, 0.5), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light1));

    PointLight light2(Point(-1, 0, 0.5), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light2));
}

// Pre: shapes y lights ya creados
// Post: carga en shapes y lights la escena de Cornell Box con luz puntual
void cb_onePL_specular_spheres(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights) {
    shapes.clear();
    lights.clear();

    // GEOMETRÍA
    Plane planoArriba(Direction(0, -1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoArriba));

    Plane planoAbajo(Direction(0, 1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoAbajo));

    Plane planoFondo(Direction(0, 0, -1), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoFondo));

    Plane planoIzquierda(Direction(1, 0, 0), 1, Color(0,0,0), Color(0.8, 0.2, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoIzquierda));

    Plane planoDerecha(Direction(-1, 0, 0), 1, Color(0,0,0), Color(0.2, 0.8, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoDerecha));

    // Esferas especulares
    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0,0,0), Color(0.8, 0.6, 0.9), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));

    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0, 0, 0), Color(0,0,0), Color(0.5, 0.9, 0.9), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaDerecha));


    // LUZ PUNTUAL
    PointLight light(Point(0, 0.5, 0), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light));
}

// Pre: 
// Post: 
void cb_top_AL_specular_spheres(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights) {
    shapes.clear();
    lights.clear();

    // GEOMETRÍA    
    Plane planoAbajo(Direction(0, 1, 0), 1, Color(0,0,0), Color(0.8,0.8,0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoAbajo));
    
    Plane planoFondo(Direction(0, 0, -1), 1, Color(0,0,0), Color(0.8,0.8,0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoFondo));
    
    Plane planoIzquierda(Direction(1, 0, 0), 1, Color(0,0,0), Color(0.8, 0.2, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoIzquierda));

    Plane planoDerecha(Direction(-1, 0, 0), 1, Color(0,0,0), Color(0.2, 0.8, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoDerecha));

    // Esferas especulares
    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0,0,0), Color(0.8, 0.6, 0.9), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));
    
    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0, 0, 0), Color(0,0,0), Color(0.5, 0.9, 0.9), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaDerecha));


    // LUZ DE ÁREA 
    Plane areaLight(Direction(0, -1, 0), 1, Color(1,1,1), Color(0,0,0), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(areaLight));
}

// Pre:
// Post:
void cb_onePL_plastic_spheres(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights) {
    shapes.clear();
    lights.clear();

    // GEOMETRÍA
    Plane planoArriba(Direction(0, -1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoArriba));

    Plane planoAbajo(Direction(0, 1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoAbajo));

    Plane planoFondo(Direction(0, 0, -1), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoFondo));

    Plane planoIzquierda(Direction(1, 0, 0), 1, Color(0,0,0), Color(0.8, 0.2, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoIzquierda));

    Plane planoDerecha(Direction(-1, 0, 0), 1, Color(0,0,0), Color(0.2, 0.8, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoDerecha));

    // Esferas "plásticas"
    //Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0.6, 0.4, 0.6), Color(0.2, 0.2, 0.3), Color(0,0,0));
    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0.704, 0.528, 0.792), Color(0.12, 0.12, 0.12), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));

    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0, 0, 0), Color(0.3, 0.5, 0.5), Color(0.2, 0.4, 0.4), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaDerecha));


    // LUZ PUNTUAL
    PointLight light(Point(0, 0.5, 0), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light));
}

// Pre:
// Post:
void cb_especular_sides(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights){
    shapes.clear();
    lights.clear();

    // GEOMETRÍA
    Plane planoArriba(Direction(0, -1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoArriba));

    Plane planoAbajo(Direction(0, 1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoAbajo));

    Plane planoFondo(Direction(0, 0, -1), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoFondo));

    Plane planoIzquierda(Direction(1, 0, 0), 1, Color(0,0,0), Color(0,0,0), Color(1, 1, 1), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoIzquierda));

    Plane planoDerecha(Direction(-1, 0, 0), 1, Color(0,0,0), Color(0,0,0), Color(1, 1, 1), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoDerecha));

    // Esferas difusas
    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0.8, 0.6, 0.9), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));

    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0, 0, 0), Color(0.5, 0.9, 0.9), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaDerecha));


    // LUZ PUNTUAL
    PointLight light(Point(0, 0.5, 0), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light));
}

// Pre:
// Post:
void cb_dielectric_spheres(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights) {
    shapes.clear();
    lights.clear();

    // GEOMETRÍA
    Plane planoArriba(Direction(0, -1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoArriba));

    Plane planoAbajo(Direction(0, 1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoAbajo));

    Plane planoFondo(Direction(0, 0, -1), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoFondo));

    Plane planoIzquierda(Direction(1, 0, 0), 1, Color(0,0,0), Color(0.8, 0.2, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoIzquierda));

    Plane planoDerecha(Direction(-1, 0, 0), 1, Color(0,0,0), Color(0.2, 0.8, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoDerecha));

    Sphere esferaIzquierda2(Point(-0.5, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0,0,0), 
                           Color(0.04, 0.04, 0.04), Color(0.96, 0.96, 0.96), 1.33);
    shapes.push_back(make_unique<Sphere>(esferaIzquierda2));

    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.15, Color(0, 0, 0), Color(0,1,1), Color(), Color(), 1.33);
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));

    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0, 0, 0), Color(0,0,0), Color(0.02, 0.02, 0.02), 
                         Color(0.98, 0.98, 0.98), 1.5);
    shapes.push_back(make_unique<Sphere>(esferaDerecha));

    // LUZ PUNTUAL
    PointLight light(Point(0, 0.5, 0), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light));
}


int main() {
    cout << "Tamaño de la imagen (anchura altura): ";
    int pixelWidth;
    cin >> pixelWidth;
    int pixelHeight;
    cin >> pixelHeight;

    int numPixels = pixelWidth * pixelHeight;

    vector<unique_ptr<GeometricShape>> shapes;
    vector<unique_ptr<PointLight>> lights;

    cb_onePL_difuse_spheres(shapes, lights);

    int opcion;
    do {
        cout << "\n=== MENÚ PRINCIPAL ===" << endl;
        cout << "Escena actual: " << current_scene << endl << endl;
        cout << "1. Luz puntual" << endl;
        cout << "2. Luz de área" << endl << endl;
        cout << "----EXTRAS----" << endl;
        cout << "3. Dos luces puntuales" << endl;
        cout << "4. Esferas especulares" << endl;
        cout << "5. Luz de área y esferas especulares" << endl;
        cout << "6. Esferas plásticas" << endl;
        cout << "7. Paredes especulares" << endl;
        cout << "8. Esferas dieléctricas" << endl;
        cout << "\n22. Generar imagen" << endl;
        cout << "0. Salir" << endl;
        cout << "Selecciona una opción: ";
        cin >> opcion;

        switch(opcion) {
            case 1: {
                if (current_scene == 1) break;
                cb_onePL_difuse_spheres(shapes, lights);
                current_scene = 1;
                break;
            }
            
            case 2: {
                if (current_scene == 2) break;
                cb_top_AL(shapes, lights);
                current_scene = 2;
                break;
            }
            
            case 3: {
                if (current_scene == 3) break;
                cb_twoPL_difuse_spheres(shapes, lights);
                current_scene = 3;
                break;
            }

            case 4: {
                if (current_scene == 4) break;
                cb_onePL_specular_spheres(shapes, lights);
                current_scene = 4;
                break;
            }

            case 5: {
                if (current_scene == 5) break;
                cb_top_AL_specular_spheres(shapes, lights);
                current_scene = 5;
                break;
            }

            case 6: {
                if (current_scene == 6) break;
                cb_onePL_plastic_spheres(shapes, lights);
                current_scene = 6;
                break;
            }

            case 7: {
                if (current_scene == 7) break;
                cb_especular_sides(shapes, lights);
                current_scene = 7;
                break;
            }

            case 8: {
                if (current_scene == 8) break;
                cb_dielectric_spheres(shapes, lights);
                current_scene = 8;
                break;
            }

            // Generar imagen con cámara configurable
            case 22: {
                if (shapes.empty()) {
                    cout << "No hay formas geométricas creadas. Agregue algunas primero." << endl;
                    break;
                }
                // Configuración spp
                cout << "SPP (actual " << raysPerPixel << " spp, presiona Enter para mantener): ";
                string sppInput;
                getline(cin, sppInput);
                getline(cin, sppInput); // Segunda llamada para capturar la línea real
                
                if (!sppInput.empty()) {
                    try {
                        int newSpp = stoi(sppInput);
                        if (newSpp > 0) {
                            raysPerPixel = newSpp;
                        }
                    } catch (const exception&) {
                        cout << "Entrada inválida, manteniendo SPP actual." << endl;
                    }
                }
                
                // Guardar la imagen
                cout << "\nNombre de la imagen (sin extensión): ";
                string nameInput;
                cin >> nameInput;
                string filenameExr = nameInput + ".exr";
                
                cout << "\n=== CONFIGURACIÓN DE CÁMARA ===" << endl;
                
                // Configurar cámara
                Camera camera(Point(0, 0, -3.5), Direction(-1,0,0), Direction(0,1,0), Direction(0,0,3));
                cout << "Cámara configurada en origen (0,0,-3.5) con vectores <l=(-1,0,0), u=(0,1,0), f=(0,0,3)>." << endl;

                cout << "\n=== GENERANDO IMAGEN ===" << endl;
                cout << "Path tracing con " << maxBounces << " rebotes máximos y " 
                     << raysPerPixel << " rayos por píxel" << endl;

                auto now = std::chrono::system_clock::now();
                std::time_t now_time = std::chrono::system_clock::to_time_t(now);
                std::tm local_time = *std::localtime(&now_time);
                cout << "Inicio: " << std::put_time(&local_time, "%H:%M:%S") << endl;
                
                // Crear la imagen
                Image image(pixelWidth, pixelHeight);
                
                // Generador de números aleatorios
                random_device rd;
                mt19937 gen(rd());
                uniform_real_distribution<double> dis(0.0, 1.0);

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
                            
                            // Trazar el rayo con path tracing recursivo
                            Color rayColor = pathTrace(ray, shapes, lights, gen, dis, 0, maxBounces, 
                                                      RR_MIN_DEPTH, RR_STOP_PROB);
                            
                            // Acumular el color de este rayo
                            total = total + rayColor;
                        }
                        
                        // Promediar los colores de todos los rayos
                        Color avg = total / static_cast<double>(raysPerPixel);

                        // Asignar el color promedio al píxel
                        image.imagen[i][j] = PixelRGB(avg.r, avg.g, avg.b);
                    }
                    
                    // Mostrar progreso cada 10% de las filas
                    if ((i + 1) % (pixelHeight / 10) == 0 || i == pixelHeight - 1) {
                        int progress = ((i + 1) * 100) / pixelHeight;
                        
                        // Obtener la hora actual
                        auto now = std::chrono::system_clock::now();

                        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

                        std::tm local_time = *std::localtime(&now_time);

                        cout << "Progreso: " << progress << "% (" << (i + 1) << "/" << pixelHeight << " filas) - "
                             << std::put_time(&local_time, "%H:%M:%S") << endl;
                    }
                }

                auto end = std::chrono::system_clock::now();
                std::time_t end_time = std::chrono::system_clock::to_time_t(end);
                std::tm end_local_time = *std::localtime(&end_time);
                cout << "Fin: " << std::put_time(&end_local_time, "%H:%M:%S") << endl;
                
                try {
                    saveHDRImage(image, filenameExr);
                    cout << "Imagen HDR guardada exitosamente como: " << filenameExr << endl;
                } catch (const exception& e) {
                    cout << "Error al guardar la imagen HDR: " << e.what() << endl;
                }
                
                break;
            }
            
            default: {
                cout << "chao" << endl;
                return 0;
            }
        }
    } while (true);
}