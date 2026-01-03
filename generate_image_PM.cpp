/**
* generate_image.cpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
* 
* Este fichero contiene la configuración de distintas escenas de Cornell Box
* además de la función principal (main) para generar imágenes
*/

#include "ray/ray.hpp"
#include "geometry/geometric_shape.hpp"
#include "geometry/sphere.hpp"
#include "geometry/plane.hpp"
#include "geometry/triangle.hpp"
#include "geometry/bsdf_utils.hpp"
#include "geometry/color.hpp"
#include "photon_mapping.hpp"
#include "imaging/imaging.hpp"
#include "light/point_light.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <limits>
#include <random>
#include <chrono>
#include <iomanip>
#include <omp.h>

using namespace std;

// Variables globales
int current_scene = 1;          // Escena actual
int raysPerPixel = 2;         // Rayos por píxel (SPP)
int numPhotons = 100000;        // Número de fotones para el mapa de fotones global
int numNeighbors = 50;         // Número de vecinos a considerar en el mapa de fotones global
int numCausticNeighbors = 100;  // Número de vecinos para estimación de cáusticas

using PhotonMap = nn::KDTree<Photon,3,PhotonAxisPositition>;

// Cornell Box básica con luz puntual arriba y esferas difusas
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

// Cornell Box con luz de área (plano superior) y esferas difusas
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
    Plane areaLight(Direction(0, -1, 0), 1, Color(0.9, 0.9, 0.9), Color(0,0,0), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(areaLight));
}

// Cornell Box con dos luces puntuales y esferas difusas
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

// Cornell Box con luz puntual y esferas especulares
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
    Color kd(0,0,0);
    char respuesta;

    cout << "Esfera izquierda con color? (S/N): ";
    cin >> respuesta;
    if (respuesta == 'S' || respuesta == 's') kd = Color(0.8, 0.6, 0.9); else kd = Color(1, 1, 1);
    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0,0,0), kd, Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));

    cout << "Esfera derecha con color? (S/N): ";
    cin >> respuesta;
    if (respuesta == 'S' || respuesta == 's') kd = Color(0.5, 0.9, 0.9); else kd = Color(1, 1, 1);
    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0, 0, 0), Color(0,0,0), kd, Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaDerecha));


    // LUZ PUNTUAL
    PointLight light(Point(0, 0.5, 0), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light));
}

// Cornell Box con luz de área (plano superior) y esferas especulares
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
    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0,0,0), Color(1,1,1), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));
    
    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0, 0, 0), Color(0,0,0), Color(1,1,1), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaDerecha));


    // LUZ DE ÁREA 
    Plane areaLight(Direction(0, -1, 0), 1, Color(1,1,1), Color(0,0,0), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(areaLight));
}

// Cornell Box con luz puntual y esferas plásticas
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
    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0.65, 0.45, 0.75), Color(0.15, 0.15, 0.15), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));

    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0, 0, 0), Color(0.35, 0.75, 0.75), Color(0.15, 0.15, 0.15), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaDerecha));


    // LUZ PUNTUAL
    PointLight light(Point(0, 0.5, 0), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light));
}

// Cornell Box con luz puntual y paredes laterales especulares
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

// Cornell Box con luz puntual y 2 esferas dieléctricas, en la de la izquierda se encuentra dentro una difusa azul
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

    // Esferas dieléctricas
    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.15, Color(0, 0, 0), Color(0,1,1), Color(), Color(), 1.33);
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));

    Sphere esferaIzquierda2(Point(-0.5, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0,0,0), 
                           Color(0.04, 0.04, 0.04), Color(0.96, 0.96, 0.96), 1.33);
    shapes.push_back(make_unique<Sphere>(esferaIzquierda2));

    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0, 0, 0), Color(0,0,0), Color(0.02, 0.02, 0.02), 
                         Color(0.98, 0.98, 0.98), 1.5);
    shapes.push_back(make_unique<Sphere>(esferaDerecha));


    // LUZ PUNTUAL
    PointLight light(Point(0, 0.5, 0), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light));
}

// Cornell box con luz puntual y esfera izquierda plastica y derecha dieléctrica
void cb_plastic_dielectric(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights){
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

    // Esferas dieléctricas
    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0,0,0), Color(0.65, 0.45, 0.75), Color(0.15, 0.15, 0.15), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));

    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0,0,0), Color(0,0,0), Color(0.02, 0.02, 0.02), Color(0.98, 0.98, 0.98), 1.5);
    shapes.push_back(make_unique<Sphere>(esferaDerecha));


    // LUZ PUNTUAL
    PointLight light(Point(0, 0.5, 0), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light));
}

// Cornell box con luz de área y esfera izquierda plastica y derecha dieléctrica
void cb_top_AL_plastic_dielectric(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights){
    shapes.clear();
    lights.clear();

    // GEOMETRÍA
    Plane planoAbajo(Direction(0, 1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoAbajo));

    Plane planoFondo(Direction(0, 0, -1), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoFondo));

    Plane planoIzquierda(Direction(1, 0, 0), 1, Color(0,0,0), Color(0.8, 0.2, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoIzquierda));

    Plane planoDerecha(Direction(-1, 0, 0), 1, Color(0,0,0), Color(0.2, 0.8, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoDerecha));

    // Esferas dieléctricas
    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0,0,0), Color(0.65, 0.45, 0.75), Color(0.15, 0.15, 0.15), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));

    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0,0,0), Color(0,0,0), Color(0.02, 0.02, 0.02), Color(0.98, 0.98, 0.98), 1.5);
    shapes.push_back(make_unique<Sphere>(esferaDerecha));

    
    // LUZ ÁREA
    Plane areaLight(Direction(0, -1, 0), 1, Color(0.9, 0.9, 0.9), Color(0,0,0), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(areaLight));
}

// Cornell box para color bleeding
void color_bleeding_scene(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights) {
    shapes.clear();
    lights.clear();

    // GEOMETRÍA
    //Plane planoArriba(Direction(0, -1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    Plane planoArriba(Direction(0, -1, 0), 1, Color(1,1,1), Color(0,0,0), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoArriba));

    Plane planoAbajo(Direction(0, 1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoAbajo));

    Plane planoFondo(Direction(0, 0, -1), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoFondo));

    Plane planoIzquierda(Direction(1, 0, 0), 1, Color(0,0,0), Color(0.8, 0.2, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoIzquierda));

    Plane planoDerecha(Direction(-1, 0, 0), 1, Color(0,0,0), Color(0.2, 0.8, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoDerecha));

    // Esfera difusa blanca en el centro
    Sphere esferaCentro(Point(0, -0.7, 0), 0.3, Color(0,0,0), Color(0.9, 0.9, 0.9), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaCentro));

    // LUZ PUNTUAL
    //PointLight light(Point(0, 0.5, 0), Color(1, 1, 1));
    //lights.push_back(make_unique<PointLight>(light));
}

// Cornell Box básica con luz puntual arriba y una esfera, donde se percibe sombras duras
void hard_shadow_scene(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights) {
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

    Sphere esfera(Point(0, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0.8, 0.6, 0.9), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esfera));


    // LUZ PUNTUAL
    PointLight light(Point(0, 0.5, 0), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light));
}

// Cornell Box básica con luz puntual arriba y una esfera, donde se percibe sombras suaves
void soft_shadow_scene(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights) {
    shapes.clear();
    lights.clear();

    // GEOMETRÍA
    Plane planoAbajo(Direction(0, 1, 0), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoAbajo));

    Plane planoFondo(Direction(0, 0, -1), 1, Color(0,0,0), Color(0.8, 0.8, 0.8), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoFondo));

    Plane planoIzquierda(Direction(1, 0, 0), 1, Color(0,0,0), Color(0.8, 0.2, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoIzquierda));

    Plane planoDerecha(Direction(-1, 0, 0), 1, Color(0,0,0), Color(0.2, 0.8, 0.2), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(planoDerecha));

    Sphere esfera(Point(0, -0.7, 0.25), 0.3, Color(0, 0, 0), Color(0.8, 0.6, 0.9), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esfera));


    // LUZ ÁREA
    Plane areaLight(Direction(0, -1, 0), 1, Color(0.9, 0.9, 0.9), Color(0,0,0), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Plane>(areaLight));
}

// Cornell box con luz puntual y esfera dieléctrica, para ver caústicas
void caustics_scene(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights){
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

    // Esferas dieléctricas
    Sphere esferaDerecha(Point(0.5, -0.7, -0.25), 0.3, Color(0,0,0), Color(0,0,0), Color(0.02, 0.02, 0.02), Color(0.98, 0.98, 0.98), 1.5);
    shapes.push_back(make_unique<Sphere>(esferaDerecha));

    Sphere esferaIzquierda(Point(-0.5, -0.7, 0.25), 0.3, Color(0,0,0), Color(0,0,0), 
                           Color(0.96, 0.96, 0.96), Color(0.04, 0.04, 0.04), 1.33);
    shapes.push_back(make_unique<Sphere>(esferaIzquierda));


    // LUZ PUNTUAL
    PointLight light(Point(0, 0.5, 0), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light));
}

// Cornell Box con 3 esferas a diferentes profundidades para demostrar Depth of Field
void dof_three_spheres_scene(vector<unique_ptr<GeometricShape>>& shapes, vector<unique_ptr<PointLight>>& lights) {
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

    // TRES ESFERAS A DIFERENTES PROFUNDIDADES (z diferente)    
    // Esfera CERCANA (más cerca de la cámara) - Roja/Magenta
    Sphere esferaCercana(Point(-0.25, -0.25, -1), 0.25, Color(0, 0, 0), Color(0.9, 0.3, 0.5), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaCercana));
    
    // Esfera MEDIA (en el centro) - Verde/Cian
    Sphere esferaCentro(Point(0, -0.25, 0.0), 0.25, Color(0, 0, 0), Color(0.3, 0.9, 0.7), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaCentro));
    
    // Esfera LEJANA (más cerca del fondo) - Azul/Violeta
    Sphere esferaLejana(Point(0.25, -0.25, 1), 0.25, Color(0, 0, 0), Color(0.4, 0.5, 0.9), Color(0,0,0), Color(0,0,0));
    shapes.push_back(make_unique<Sphere>(esferaLejana));


    // LUZ PUNTUAL
    PointLight light(Point(0, 0.7, 0), Color(1, 1, 1));
    lights.push_back(make_unique<PointLight>(light));
}


int main() {
    cout << "Tamaño de la imagen (anchura altura): ";
    int pixelWidth;
    cin >> pixelWidth;
    int pixelHeight;
    cin >> pixelHeight;

    vector<unique_ptr<GeometricShape>> shapes;
    vector<unique_ptr<PointLight>> lights;

    cb_onePL_difuse_spheres(shapes, lights);

    int opcion;
    do {
        cout << "\n=== MENÚ PRINCIPAL ===" << endl;
        cout << "Escena actual: " << current_scene << endl << endl;
        cout << "1.  Luz puntual" << endl;
        cout << "2.  Luz de área" << endl << endl;
        cout << "----EXTRAS----" << endl;
        cout << "3.  Dos luces puntuales" << endl;
        cout << "4.  Esferas especulares" << endl;
        cout << "5.  Luz de área y esferas especulares" << endl;
        cout << "6.  Esferas plásticas" << endl;
        cout << "7.  Paredes especulares" << endl;
        cout << "8.  Esferas dieléctricas" << endl;
        cout << "9.  Esferas plástica y dieléctrica" << endl;
        cout << "10. Esferas plástica y dieléctrica, con luz de área" << endl;
        cout << "11. Color Bleeding" << endl;
        cout << "12. Sombras duras" << endl;
        cout << "13. Sombras suaves" << endl;
        cout << "14. Caústicas" << endl;
        cout << "15. Depth of Field - 3 esferas a diferentes profundidades" << endl;
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
                cb_onePL_specular_spheres(shapes, lights);
                current_scene = 4;
                break;
            }

            case 5: {
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

            case 9: {
                if (current_scene == 9) break;
                cb_plastic_dielectric(shapes, lights);
                current_scene = 9;
                break;
            }

            case 10: {
                if (current_scene == 10) break;
                cb_top_AL_plastic_dielectric(shapes, lights);
                current_scene = 10;
                break;
            }

            case 11: {
                if (current_scene == 11) break;
                color_bleeding_scene(shapes, lights);
                current_scene = 11;
                break;
            }

            case 12: {
                if (current_scene == 11) break;
                hard_shadow_scene(shapes, lights);
                current_scene = 12;
                break;
            }

            case 13: {
                if (current_scene == 11) break;
                soft_shadow_scene(shapes, lights);
                current_scene = 12;
                break;
            }

            case 14: {
                if (current_scene == 14) break;
                caustics_scene(shapes, lights);
                current_scene = 14;
                break;
            }

            case 15: {
                if (current_scene == 15) break;
                dof_three_spheres_scene(shapes, lights);
                current_scene = 15;
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

                cout << "Número de fotones GLOBALES a generar (actual " << numPhotons << ", presiona Enter para mantener): ";
                string numPhotonsInput;
                getline(cin, numPhotonsInput);
                if (!numPhotonsInput.empty()) {
                    try {
                        int newNumPhotons = stoi(numPhotonsInput);
                        if (newNumPhotons > 0) {
                            numPhotons = newNumPhotons;
                        }
                    } catch (const exception&) {
                        cout << "Entrada inválida, manteniendo número de fotones globales actual." << endl;
                    }
                }

                numNeighbors = static_cast<int>(numPhotons * 0.007); // Valor recomendado por defecto

                cout << "Número de vecinos (k) para GLOBALES, ";
                cout << "(recomendado y actual " << numNeighbors << ", presiona Enter para mantener): ";
                string numNeighborsInput;
                getline(cin, numNeighborsInput);
                if (!numNeighborsInput.empty()) {
                    try {
                        int newNumNeighbors = stoi(numNeighborsInput);
                        if (newNumNeighbors > 0) {
                            numNeighbors = newNumNeighbors;
                        }
                    } catch (const exception&) {
                        cout << "Entrada inválida, manteniendo número de vecinos globales actual." << endl;
                    }
                }

                numCausticNeighbors = static_cast<int>(numPhotons * 0.003); // Valor recomendado

                cout << "Número de vecinos (k) para CÁUSTICAS, ";
                cout << "(recomendado y actual " << numCausticNeighbors << ", presiona Enter para mantener): ";
                string numCausticNeighborsInput;
                getline(cin, numCausticNeighborsInput);
                if (!numCausticNeighborsInput.empty()) {
                    try {
                        int newNumCausticNeighbors = stoi(numCausticNeighborsInput);
                        if (newNumCausticNeighbors > 0) {
                            numCausticNeighbors = newNumCausticNeighbors;
                        }
                    } catch (const exception&) {
                        cout << "Entrada inválida, manteniendo número de vecinos de cáusticas actual." << endl;
                    }
                }

                cout << "¿Quieres utilizar Next Event Estimation? (s/n): ";
                string neeInput;
                getline(cin, neeInput);
                bool useNEE = (neeInput == "s" || neeInput == "S");

                cout << "Kernel a utilizar (1. Caja | 2. Cono | 3. Gaussiano): ";
                int kernelChoice;
                cin >> kernelChoice;

                cout << "\nNombre de la imagen (sin extensión): ";
                string nameInput;
                cin >> nameInput;
                string filenameExr = nameInput + ".exr";
                
                cout << "\n=== CONFIGURACIÓN DE CÁMARA ===" << endl;
                
                // Preguntar si usar profundidad de campo
                cout << "¿Usar profundidad de campo (DoF)? (s/n): ";
                string dofInput;
                cin >> dofInput;
                bool useDoF = (dofInput == "s" || dofInput == "S");
                
                Camera camera(Point(0, 0, -3.5), Direction(-1,0,0), Direction(0,1,0), Direction(0,0,3));
                int apertureChoice;

                if (useDoF) {
                    cout << "Radio de apertura (ej. 0.1): ";
                    double apertureRadius;
                    cin >> apertureRadius;
                    
                    cout << "Distancia focal (distancia al plano enfocado, ej. 3.5): ";
                    double focalDistance;
                    cin >> focalDistance;

                    cout << "Tipo de apertura a utilizar (0. Circular | 1. Cuadrada): ";
                    cin >> apertureChoice;
                    
                    camera = Camera(Point(0, 0, -3.5), Direction(-1,0,0), Direction(0,1,0), 
                                   Direction(0,0,3), apertureRadius, focalDistance);
                    
                    cout << "Cámara con DoF configurada:" << endl;
                    cout << "  - Origen: (0,0,-3.5)" << endl;
                    cout << "  - Radio apertura: " << apertureRadius << endl;
                    cout << "  - Distancia focal: " << focalDistance << endl;
                    cout << "  - Longitud focal calculada: " << camera.focalLength << endl;
                } else {
                    cout << "Cámara pinhole configurada en origen (0,0,-3.5) con vectores <l=(-1,0,0), u=(0,1,0), f=(0,0,3)>." << endl;
                }

                cout << "\n=== GENERANDO IMAGEN ===" << endl;
                cout << "Parámetros:" << endl;
                cout << "  - Fotones GLOBALES: " << numPhotons << endl;
                cout << "  - K vecinos para globales: " << numNeighbors << endl;
                cout << "  - K vecinos para cáusticas: " << numCausticNeighbors << endl;
                cout << "  - Caminos por píxel (SPP): " << raysPerPixel << endl;
                cout << "  - Next Event Estimation: " << (useNEE ? "Sí" : "No") << endl;
                cout << "  - Kernel: ";
                switch (kernelChoice) {
                    case 1: cout << "Caja"; break;
                    case 2: cout << "Cono"; break;
                    case 3: cout << "Gaussiano"; break;
                    default: cout << "Desconocido"; break;
                }
                cout << endl;

                // Inicio del tiempo de generación
                auto start_time = chrono::high_resolution_clock::now();
                auto now = chrono::system_clock::now();
                time_t now_time = chrono::system_clock::to_time_t(now);
                tm local_time = *localtime(&now_time);
                cout << "Inicio: " << put_time(&local_time, "%H:%M:%S") << endl;
                
                // Crear la imagen
                Image image(pixelWidth, pixelHeight);
                
                // Generador de números aleatorios
                random_device rd;
                atomic<int> filasProcesadas = 0;

                // Paso 1: Generar mapa de fotones lanzando rayos desde la cámara
                cout << "\n--- PASO 1: Generando mapas de fotones ---" << endl;
                auto photon_start = chrono::high_resolution_clock::now();

                PhotonMap global_map, caustic_map;

                pair<PhotonMap, PhotonMap> maps = generate_photon_map(numPhotons, shapes, lights, useNEE);
                global_map = maps.first;
                caustic_map = maps.second;

                auto photon_end = chrono::high_resolution_clock::now();
                chrono::duration<double> photon_duration = photon_end - photon_start;
                cout << "Mapa de fotones generado en " << fixed << setprecision(2) 
                     << photon_duration.count() << " segundos" << endl;

                // Paso 2: Renderizar la imagen con path tracing y el mapa de fotones
                cout << "\n--- PASO 2: Renderizando imagen ---" << endl;
                auto render_start = chrono::high_resolution_clock::now();

                #pragma omp parallel
                {
                    // De momento determinista (TODO: pensar en inclusión de rd())
                    mt19937 gen(1234 + omp_get_thread_num()); // Semilla diferente por hilo
                    uniform_real_distribution<double> dis(0.0, 1.0);
                    double aperture_x, aperture_y;  // Para muestreo en apertura

                    #pragma omp for schedule(dynamic)
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
                                
                                Point rayOrigin;
                                Direction rayDirection;
                                
                                if (camera.apertureRadius > 0.0) {
                                    // CÁMARA CON PROFUNDIDAD DE CAMPO
                                    
                                    // 1. Calcular la dirección hacia el píxel
                                    Direction pixelDirection(x, y, z);
                                    
                                    // 2. El punto focal está a focalDistance en la dirección del píxel
                                    // Esta es la distancia que el usuario especificó como "distancia al plano enfocado"
                                    Point focalPoint = Point(0,0,0) + pixelDirection.normalized() * camera.focalDistance;
                                    
                                    // 3. Muestrear un punto aleatorio en la apertura
                                    if (apertureChoice) {
                                        // Apertura circular
                                        // Muestreo uniforme en disco usando el método de Shirley
                                        double r = camera.apertureRadius * sqrt(dis(gen));
                                        double theta = 2.0 * M_PI * dis(gen);
                                        aperture_x = r * cos(theta);
                                        aperture_y = r * sin(theta);
                                    } else {
                                        // Apertura cuadrada
                                        // Muestreo uniforme de x e y entre [-1, 1]
                                        aperture_x = (2.0 * dis(gen) - 1.0) * camera.apertureRadius;
                                        aperture_y = (2.0 * dis(gen) - 1.0) * camera.apertureRadius;
                                    }
                                    // Punto de origen en coordenadas de cámara (la apertura está en z=0)
                                    Point cameraOrigin(aperture_x, aperture_y, 0.0);
                                    
                                    // 4. La dirección del rayo va desde el punto en la apertura al punto focal
                                    Direction cameraDirection = focalPoint - cameraOrigin;
                                    
                                    // 5. Transformar al espacio mundo
                                    rayOrigin = camera.cameraToWorld(cameraOrigin);
                                    rayDirection = camera.cameraToWorld(cameraDirection);
                                    
                                } else {
                                    // CÁMARA PINHOLE (sin profundidad de campo)
                                    Point cameraPixelPoint(x, y, z);
                                    Point cameraLocalOrigin = Point(0,0,0);
                                    Direction cameraDirection = cameraPixelPoint - cameraLocalOrigin;
                                    rayDirection = camera.cameraToWorld(cameraDirection);
                                    rayOrigin = camera.origin;
                                }
                                
                                // Crear el rayo 
                                Ray ray(rayOrigin, rayDirection.normalized());
                                
                                // Trazar el rayo con photon mapping recursivo
                                Color rayColor = photonMap(ray, shapes, lights, 0, global_map, caustic_map, 
                                                        useNEE, kernelChoice, numNeighbors);
                                
                                // Acumular el color de este rayo
                                total = total + rayColor;
                            }
                            
                            // Promediar los colores de todos los rayos
                            Color avg = total / static_cast<double>(raysPerPixel);

                            // Asignar el color promedio al píxel
                            image.imagen[i][j] = PixelRGB(avg.r, avg.g, avg.b);
                        }
                        
                        // Un hilo muestra progreso cada 10% de las filas
                        int f = ++filasProcesadas;       // atómico, seguro
                        if (f % (pixelHeight / 10) == 0 || f == pixelHeight - 1) {
                            #pragma omp critical
                            {
                                // Obtener la hora actual
                                auto now = std::chrono::system_clock::now();

                                std::time_t now_time = std::chrono::system_clock::to_time_t(now);

                                std::tm local_time = *std::localtime(&now_time);

                                cout << "Progreso: " << (f * 100) / pixelHeight << "% (" << f << "/" << pixelHeight << " filas) - "
                                    << std::put_time(&local_time, "%H:%M:%S") << endl;
                            }
                        }
                    }
                }

                auto render_end = chrono::high_resolution_clock::now();
                chrono::duration<double> render_duration = render_end - render_start;
                cout << "Renderizado completado en " << fixed << setprecision(2) 
                     << render_duration.count() << " segundos" << endl;

                auto end_time = chrono::high_resolution_clock::now();
                auto end = chrono::system_clock::now();
                time_t end_time_t = chrono::system_clock::to_time_t(end);
                tm end_local_time = *localtime(&end_time_t);
                cout << "\nFin: " << put_time(&end_local_time, "%H:%M:%S") << endl;
                
                // Calcular duración total
                chrono::duration<double> duration = end_time - start_time;
                double elapsed_seconds = duration.count();
                
                cout << fixed << setprecision(4);
                cout << "Tiempo total: " << elapsed_seconds << " segundos" << endl;
                cout << "  - Generación fotones: " << photon_duration.count() << " s" << endl;
                cout << "  - Renderizado: " << render_duration.count() << " s" << endl;

                // Guardar la imagen HDR y LDR
                try {
                    saveHDRImage(image, filenameExr);
                    cout << "Imagen HDR guardada como: " << filenameExr << endl;
                } catch (const exception& e) {
                    cout << "Error al guardar la imagen HDR: " << e.what() << endl;
                }
                
                string filenamePng;
                try {
                    Image ldr_image = gamma_curve(image, 2.2);
                    filenamePng = nameInput + ".png";
                    savePNGImage(ldr_image, filenamePng);
                } catch (const exception& e) {
                    cout << "Error al guardar la imagen LDR: " << e.what() << endl;
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