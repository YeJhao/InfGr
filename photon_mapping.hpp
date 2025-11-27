#ifndef PHOTON_MAPPING_HPP
#define PHOTON_MAPPING_HPP

#include "ray.hpp"
#include "kdTree/kdtree.h"
#include "geometry/geometric_shape.hpp"
#include "geometry/color.hpp"
#include "geometry/point.hpp"
#include "geometry/direction.hpp"
#include "path_tracing.hpp"
#include <vector>
#include <cmath>
#include <list>

using namespace std;

// Variables de configuración
#define PHOTON_MIN_DEPTH 2

// Variables globales
//uniform_real_distribution<double> dis0 = uniform_real_distribution<double>(0.0, 1.0);
//uniform_real_distribution<double> dis1 = uniform_real_distribution<double>(-1.0, 1.0);
//gen = mt19937(random_device{}());
mt19937 gen(random_device{}());
uniform_real_distribution<double> dis0(0.0, 1.0);
uniform_real_distribution<double> dis1(-1.0, 1.0);


class Photon {
    public:
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
            photon.flux_ = flux;
            photon.position_ = hit.point;     
            photon.direction_ = ray.d; // Dirección incidente
            photon_list.push_back(photon);
        }
    } else {
        // Si no hay intersección, terminar
        return;
    }

    double pKill, pDiff, pSpec, pTrans;
    calculateProbabilities(hit, pKill, pDiff, pSpec, pTrans);

    //Ruleta rusa a partir de PHOTON_MIN_DEPTH
    if (depth >= PHOTON_MIN_DEPTH) {
        double random = dis0(gen);
        if(random < pKill) {
            return; // Terminar el trazado del fotón
        }
    }

    // Si no terminamos el camino, calculamos el siguiente rayo y el nuevo flujo
    // Modelar siguiente dirección del fotón según el material (lóbulo difuso, especular o refractivo)
    Direction newRayDir;
    Color throughput; // Factor de transmisión del fotón

    calculateThroughput(hit, pDiff, pSpec, pTrans, dis0(gen), newRayDir, throughput);

    // Crear nuevo rayo
    Ray newRay(hit.point, newRayDir);

    // Calcular nuevo flujo del fotón
    Color newFlux = Color();
    newFlux = throughput * flux; // Lo que transmite el material * flujo entrante

    // Llamada recursiva
    recursive_trace_photon(depth + 1, newRay, newFlux, shapes, photon_list);
}


inline Direction sampleDirectionFromPointLight() {
    // Generación de números aleatorios
    double u0 = dis0(gen);
    double u1 = dis1(gen);

    double phi = 2.0 * M_PI * u0;
    double theta = acos(u1);

    double sinTheta = sin(theta);
    double x = sinTheta * cos(phi);
    double y = sinTheta * sin(phi);
    double z = cos(theta);
    
    return Direction(x, y, z);
}

inline PhotonMap generate_photon_map(const int numRays,
                        const vector<unique_ptr<GeometricShape>>& shapes,
                        const vector<unique_ptr<PointLight>>& lights
        ){
    list<Photon> photons;
    double doubleNumRays = static_cast<double>(numRays);
    double doubleLights = static_cast<double>(lights.size());
    double raysPerLight = doubleNumRays / doubleLights;
    for (auto light: lights) {
        Color initialFlux = light->intensity * 4.0 * M_PI / raysPerLight;
        for (int i = 0; i < floor(raysPerLight); ++i) {
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

    return photon_map;
}

inline Color photonMap(const Ray& ray, 
                       const vector<unique_ptr<GeometricShape>>& shapes,
                       const vector<unique_ptr<PointLight>>& lights,
                       int depth,
                       const PhotonMap& photon_map,
                       const int k = 50) {
    // Límite de seguridad (por si acaso)
    if (depth >= MAX_BOUNCES) {
        return Color(0, 0, 0);
    }
    
    // Encontrar intersección más cercana
    HitInfo hit = findClosestIntersection(ray, shapes, ray.o);
    
    // Si no hay intersección, devolver color de fondo
    if (!hit.hit) {
        return Color(0, 0, 0);
    }

    // Si el objeto emite luz, devolver la emisión (no rebota)
    if (hit.emission.r > 0 || hit.emission.g > 0 || hit.emission.b > 0) {
        return hit.emission;
    }

    // Color acumulado (iluminación directa + indirecta)
    Color L(0, 0, 0);

    // ============================================
    // 1. ILUMINACIÓN DIRECTA (Direct Lighting)
    // ============================================
    for (const auto& light : lights) {
        // Comprobar visibilidad con shadow ray
        if (isVisible(hit.point, light->position, shapes)) {
            Direction wi = (light->position - hit.point);
            double distToLight = wi.norm();
            wi = wi / distToLight; // Normalizar
            
            // Atenuación por distancia
            Color Li = light->intensity / (distToLight * distToLight);
            
            // BRDF difusa (solo difusa para iluminación directa)
            Color fr = hit.kd * (1.0 / M_PI);

            // Factor geométrico
            double cosTheta = max(0.0, hit.normal.dot(wi));
            
            // Contribución de esta luz
            L = L + (Li * fr * cosTheta);
        }
    }

    // ============================================
    // 2. ILUMINACIÓN INDIRECTA (Indirect Lighting)
    // ============================================
    double pKill, pDiff, pSpec, pTrans;
    calculateProbabilities(hit, pKill, pDiff, pSpec, pTrans);

    // Solo calculada cuando el material es no delta (difuso)
    if (isNonDeltaMaterial(hit)) {
        vector<const Photon*> nearestPhotons = photon_map.nearest_neighbors(hit.point, k);

        Color indirectLight(0, 0, 0);

        // Calculamos el radio máximo entre los k fotones más cercanos
        float maxDist = 0.0f;
        for (const Photon* photon : nearestPhotons) {
            float dist = (photon->position_ - hit.point).norm();
            maxDist = max(maxDist, dist);
        }

        // Denominador de la estimación del mapa de fotones
        float area = M_PI * maxDist * maxDist;

        // BRDF difusa
        Color fr = hit.kd / (M_PI * pDiff);

        for (const Photon* photon : nearestPhotons) {            
            // Contribución += BSDF * flujo del fotón / área
            indirectLight = indirectLight + (fr * photon->flux_ / area);
        }

    } else {
        double rrValue = dis(gen);
        if (rrValue < pSpec) {
            // ===== LÓBULO ESPECULAR (Reflexión perfecta) =====
            Direction wo = ray.d * (-1.0); // Dirección hacia afuera (opuesta al rayo incidente)
            Direction newRayDir = perfectReflection(wo, hit.normal);

            Ray newRay(hit.point, newRayDir);
            return L + (photonMap(newRay, shapes, lights, depth + 1, photon_map, k) / pSpec);

        } else {
            // ===== LÓBULO DE TRANSMISIÓN (Refracción) =====
            Direction wo = ray.d * (-1.0); // Dirección hacia afuera
            
            // Determinar si estamos entrando o saliendo del objeto
            // Si n·wo > 0, el rayo viene del exterior (entrando)
            // Si n·wo < 0, el rayo viene del interior (saliendo)
            double cosTheta = hit.normal.dot(wo);
            
            // IORs por defecto (estos valores deberían venir de las geometrías en el futuro)
            double iorFrom, iorTo;
            Direction effectiveNormal;
            
            if (cosTheta > 0) {
                // Entrando al objeto: aire -> material
                iorFrom = 1.0;  // IOR del aire
                iorTo = hit.ior;    // IOR del material
                effectiveNormal = hit.normal;
            } else {
                // Saliendo del objeto: material -> aire
                iorFrom = hit.ior;  // IOR del material
                iorTo = 1.0;    // IOR del aire
                effectiveNormal = hit.normal * (-1.0); // Invertir la normal
                cosTheta = -cosTheta; // Corregir el coseno
            }
            
            // Calcular dirección refractada
            Direction wt = perfectRefraction(wo, effectiveNormal, iorFrom, iorTo);
            
            // Comprobar si hubo reflexión interna total
            if (wt.d[0] == 0 && wt.d[1] == 0 && wt.d[2] == 0) {
                // Reflexión interna total - usar reflexión en lugar de refracción
                newRayDir = perfectReflection(wo, effectiveNormal);
            } else {
                // Refracción exitosa
                newRayDir = wt;
            }

            Ray newRay(hit.point, newRayDir);

            return L + (photonMap(newRay, shapes, lights, depth + 1, photon_map, k) / pTrans);
        }
    }
    
    return L + indirectLight;
}


#endif // PHOTON_MAPPING_HPP