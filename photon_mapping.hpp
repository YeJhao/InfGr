#ifndef PHOTON_MAPPING_HPP
#define PHOTON_MAPPING_HPP

#include "ray.hpp"
#include "kdTree/kdtree.h"
#include "geometry/geometric_shape.hpp"
#include "geometry/color.hpp"
#include "geometry/point.hpp"
#include "geometry/direction.hpp"
#include <vector>
#include <cmath>

using namespace std;

// Variables de configuración
#define PHOTON_MIN_DEPTH 2

// Variables globales
uniform_real_distribution<double> dis0 = uniform_real_distribution<double>(0.0, 1.0);
uniform_real_distribution<double> dis1 = uniform_real_distribution<double>(-1.0, 1.0);
gen = mt19937(random_device{}());


class Photon {
    Point position_;        // 3D point of the interaction
    Direction direction_;   // Incident direction of the photon
    Color flux_;            // Flux (power) of the photon

    float position(std::size_t i) const { return position_[i]; }
};

struct PhotonAxisPositition {
    float operator()(const Photon& p, std::size_t i) const {
        return p.position(i);
    }
};

using PhotonMap = nn::KDTree<Photon,3,PhotonAxisPositition>;

inline bool isNonDeltaMaterial(const HitInfo& hit) {
    // Comprobar si el material tiene componente difusa (kd) o si no es completamente especular/refractiva
    return (hit.kd.r > 0 || hit.kd.g > 0 || hit.kd.b > 0);
}

inline void recursive_trace_photon(const int depth, 
                        const Ray& ray,
                        const Color& flux,
                        const vector<unique_ptr<GeometricShape>>& shapes,
                        list<Photon>& photon_list
                        ){

    // Encontrar intersección más cercana con la geometría
    HitInfo hit = findClosestIntersection(ray, shapes, ray.o);

    // Comprobar si la intersección es con geometría NO delta (especular/refractiva)
    // Entonces, guardar el fotón en el mapa de fotones
    if (hit.hit) {
        if (isNonDeltaMaterial(hit)) {
            // Guardar fotón en el mapa de fotones
            Photon photon;
            photon.position_ = hit.point;     
            photon.direction_ = ray.d; // Dirección incidente       
        }
    } else {
        // Si no hay intersección, terminar
        return;
    }

    if (depth >= PHOTON_MIN_DEPTH) {
        //Ruleta rusa a partir de PHOTON_MIN_DEPTH
    }

    // Modelar siguiente dirección del fotón según el material (lóbulo difuso, especular o refractivo)
    Direction newRayDir;

    // Crear nuevo rayo

    // Llamada recursiva
    recursive_trace_photon(depth + 1, newRay, newFlux, shapes, photon_list);
}


inline Direction sampleDirectionFromPointLight() {
    // Generación de números aleatorios
    double u0 = dis0(gen);
    double u1 = dis1(gen);

    double phi = 2.0 * M_PI * u0;
    double theta = acos(u1);

    doube sinTheta = sin(theta);
    double x = sinTheta * cos(phi);
    double y = sinTheta * sin(phi);
    double z = cos(theta);
    
    return Direction(x, y, z);
}

inline void generate_photon_map(const int numRays,
                        const vector<unique_ptr<GeometricShape>>& shapes,
                        const vector<unique_ptr<PointLight>>& lights
        ){
    list<Photon> photons;
    for (auto light: lights) {
        Color initialFlux = light->intensity * 4.0 * M_PI / static_cast<float>(numRays/lights.size());
        for (int i = 0; i < floor(numRays/lights.size()); ++i) {
            // Lanzar dirección aleatoria desde la luz 
            Direction dir = sampleDirectionFromPointLight();
            Ray ray(light->position, dir);
            // Llamar recursive_trace_photon con
            // - Profundidad 0
            // - Rayo desde la luz en la dirección muestreada
            // - Flujo igual a la intensidad de la luz
            // - Geometrías de la escena
            recursive_trace_photon(0, ray, initialFlux, shapes, photons);
        }
    }
    // Crear el mapa de fotones con la lista de fotones generada
    PhotonMap photon_map(photons, PhotonAxisPositition());
}


#endif // PHOTON_MAPPING_HPP