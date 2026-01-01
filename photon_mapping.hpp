#ifndef PHOTON_MAPPING_HPP
#define PHOTON_MAPPING_HPP

#include "ray/ray.hpp"
#include "kdTree/kdtree.h"
#include "geometry/geometric_shape.hpp"
#include "geometry/color.hpp"
#include "geometry/bsdf_utils.hpp"
#include "light/point_light.hpp"
#include "path_tracing.hpp"
#include <vector>
#include <cmath>
#include <list>
#include <array>
#include <chrono>
#include <iomanip>

using namespace std;

// Variables de configuración
#define PHOTON_MIN_DEPTH 2
#define ALPHA 0.918
#define BETA 1.953

// Variables globales
mt19937 gen(random_device{}());
uniform_real_distribution<double> dis0(0.0, 1.0);
uniform_real_distribution<double> dis1(-1.0, 1.0);


class Photon {
    public:
        Point position_;        // Punto de la intersección del fotón
        Direction direction_;   // Dirección incidente del fotón
        Color flux_;            // Flujo (potencia) del fotón

        int prof_;              // Profundidad del fotón en el trazado
        bool isCaustic_;        // Indica si el fotón es parte de una cáustica

        float position(size_t i) const { 
            return static_cast<float>(position_.coords(i)); 
        }
};

struct PhotonAxisPositition {
    float operator()(const Photon& p, size_t i) const {
        return p.position(i);
    }
};

using PhotonMap = nn::KDTree<Photon,3,PhotonAxisPositition>;

// Función auxiliar para convertir Point a array<float, 3>
inline array<float, 3> pointToArray(const Point& p) {
    return {static_cast<float>(p.coords(0)), 
            static_cast<float>(p.coords(1)), 
            static_cast<float>(p.coords(2))};
}

// Función auxiliar para convertir array a Point
inline Point arrayToPoint(const array<float, 3>& arr) {
    return Point(static_cast<double>(arr[0]), 
                 static_cast<double>(arr[1]), 
                 static_cast<double>(arr[2]));
}

inline bool isNonDeltaMaterial(const HitInfo& hit) {
    // Comprobar si el material tiene componente difusa (kd) o si no es completamente especular/refractiva
    return (hit.kd.r > 0 || hit.kd.g > 0 || hit.kd.b > 0);
}

inline void recursive_trace_photon(const int depth, 
                        const Ray& ray,
                        const vector<unique_ptr<GeometricShape>>& shapes,
                        const Color& flux,
                        list<Photon>& photon_list,
                        const int maxPhotonsPerLight,
                        const bool useNEE,
                        bool hasBouncedonDelta = false
    ){
    // Encontrar intersección más cercana con la geometría
    HitInfo hit = findClosestIntersection(ray, shapes, ray.o);

    // Comprobar si la intersección es con geometría NO delta (especular/refractiva)
    // Entonces, guardar el fotón en el mapa de fotones
    if (hit.hit) {
        if (useNEE && depth == 0) { 
            // Si utilizamos NEE y es el primer rebote, no guardamos el fotón
        } else if (isNonDeltaMaterial(hit)) {
            // Guardar fotón en el mapa de fotones
            Photon photon;
            photon.position_ = hit.point;     
            photon.direction_ = ray.d; // Dirección incidente
            photon.isCaustic_ = hasBouncedonDelta;
            photon.flux_ = flux;
            photon.prof_ = depth;
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

    calculateThroughput(hit, ray, pDiff, pDiff + pSpec, dis0(gen), newRayDir, throughput, gen, dis0);

    // Crear nuevo rayo
    Ray newRay(hit.point, newRayDir);

    if (photon_list.size() >= static_cast<size_t>(maxPhotonsPerLight)) {
        return; // Hemos alcanzado el número máximo de fotones por luz
    }

    // Marcar que rebotó en delta si no es difuso
    bool nextHasBounced = hasBouncedonDelta || (hit.kt.r > 0 || hit.kt.g > 0 || hit.kt.b > 0);

    // Llamada recursiva
    recursive_trace_photon(depth + 1, newRay, shapes, throughput * flux, 
                           photon_list, maxPhotonsPerLight, useNEE, nextHasBounced);
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

inline void calculatePhotonsFlux(const int numRays,
                                 list<Photon>& photons
){    
    // Normalización en base a número de rayos lanzados desde la luz
    double normalization = 4.0 * M_PI / static_cast<double>(numRays);

    // Actualizar el flujo de cada fotón en la lista
    for (auto& photon: photons) {
        photon.flux_ = photon.flux_ * normalization;
    }
}

inline pair<PhotonMap, PhotonMap> generate_photon_map(const int numPhotons,
                        const vector<unique_ptr<GeometricShape>>& shapes,
                        const vector<unique_ptr<PointLight>>& lights,
                        const bool useNEE
){
    list<Photon> allPhotons;    
    double photonsPerLight = numPhotons / lights.size();
    
    int photonsGenerated = 0;
    int lastProgressPercent = 0;

    cout << "Generando " << numPhotons << " fotones desde " << lights.size() 
         << (lights.size() == 1 ? " luz..." : " luces...") << endl;

    for (const auto& light: lights) {
        list<Photon> photonsAux;
        int numRays = 0;
        while (photonsGenerated < photonsPerLight) { 
            // Lanzar dirección aleatoria desde la luz 
            Direction dir = sampleDirectionFromPointLight();
            Ray ray(light->position, dir);
            
            // Aumentamos el nº de rayos lanzados por esa luz
            numRays++;

            // Llamar recursive_trace_photon con
            // - Profundidad 0
            // - Rayo desde la luz en la dirección muestreada
            // - Geometrías de la escena
            // - Intensidad de la luz como flujo inicial
            // - Lista de fotones donde almacenar los fotones generados
            // - Nº de fotones máximos a generar por luz
            // - Uso de Next Event Estimation
            recursive_trace_photon(0, ray, shapes, light->intensity, photonsAux, photonsPerLight, useNEE);

            photonsGenerated = photonsAux.size() + allPhotons.size();
            
            // Actualizar progreso
            int currentProgress = (photonsGenerated * 100) / numPhotons;
            
            // Mostrar progreso cada 10%
            if (currentProgress >= lastProgressPercent + 10) {
                lastProgressPercent = currentProgress;
                
                auto now = chrono::system_clock::now();
                time_t now_time = chrono::system_clock::to_time_t(now);
                tm local_time = *localtime(&now_time);
                
                cout << "  Progreso fotones: " << currentProgress << "% (" 
                     << photonsGenerated << "/" << numPhotons << ") - "
                     << put_time(&local_time, "%H:%M:%S") << endl;
            }
        }
        // Calculamos el flujo de cada fotón generado por esta luz
        calculatePhotonsFlux(numRays, photonsAux);

        // Añadir los fotones generados por esta luz a la lista global
        allPhotons.insert(allPhotons.end(), photonsAux.begin(), photonsAux.end());
    }

    // Separar fotones
    list<Photon> causticPhotons;
    list<Photon> globalPhotons;
    
    for (const auto& photon: allPhotons) {
        if (photon.isCaustic_) {
            causticPhotons.push_back(photon);
        } else {
            globalPhotons.push_back(photon);
        }
    }
    
    cout << "Fotones de cáusticas: " << causticPhotons.size() << endl;
    cout << "Fotones globales: " << globalPhotons.size() << endl;
    cout << "Total de fotones almacenados en el mapa: " << allPhotons.size() << endl;
    cout << "Algun foton de prof 0?: " << (any_of(allPhotons.begin(), allPhotons.end(), [](const Photon& p){ return p.prof_ == 0; }) ? "Sí" : "No") << endl;

    // Crear los mapas de fotones
    PhotonMap caustic_map(causticPhotons, PhotonAxisPositition());
    PhotonMap global_map(globalPhotons, PhotonAxisPositition());
    
    return {caustic_map, global_map};
}

inline Color nextEventEstimation(const HitInfo& hit,
                                 const vector<unique_ptr<PointLight>>& lights,
                                 const vector<unique_ptr<GeometricShape>>& shapes) 
{
    Color L(0, 0, 0);                                
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
    return L;
}

inline Color boxKernel(HitInfo& hit,
                       const vector<const Photon*>& nearestPhotons,
                       double pDiff) {
    Color indirectLight(0, 0, 0);

    if (nearestPhotons.empty()) {
        return indirectLight;
    }

    // Calculamos el radio máximo entre los k fotones más cercanos
    float maxDist = 0.0f;
    for (const Photon* photon: nearestPhotons) {
        float dist = (photon->position_ - hit.point).norm();
        maxDist = max(maxDist, dist);
    }

    // Denominador de la estimación del mapa de fotones
    float area = M_PI * maxDist * maxDist;

    // BRDF difusa
    Color fr(0, 0, 0);
    fr = hit.kd / (M_PI * pDiff);

    for (const Photon* photon: nearestPhotons) {
        // Dirección del fotón (incidente)
        Direction wi = photon->direction_ * (-1.0); // Invertir para obtener dirección que nos interesa
        
        // Factor geométrico (coseno del ángulo de incidencia)
        double cosTheta = max(0.0, hit.normal.dot(wi));
        
        // Contribución += BSDF * cosTheta * flujo del fotón / área
        indirectLight = indirectLight + (fr * cosTheta * photon->flux_ / area);
    }

    return indirectLight;
}

inline Color triangleKernel(HitInfo& hit,
                           const vector<const Photon*>& nearestPhotons,
                           double pDiff,
                           const float k = 1.0f) {
    Color indirectLight(0, 0, 0);

    if (nearestPhotons.empty()) {
        return indirectLight;
    }

    // Calculamos el radio máximo entre los k fotones más cercanos
    float maxDist = 0.0f;
    for (const Photon* photon: nearestPhotons) {
        float dist = (photon->position_ - hit.point).norm();
        maxDist = max(maxDist, dist);
    }

    // BRDF difusa
    Color fr(0, 0, 0);
    fr = hit.kd / (M_PI * pDiff);

    for (const Photon* photon: nearestPhotons) {
        // Dirección del fotón (incidente)
        Direction wi = photon->direction_ * (-1.0);
                
        double cosTheta = max(0.0, hit.normal.dot(wi));
        
        float dist = (photon->position_ - hit.point).norm();
        float weight = 1.0f - (dist / (maxDist*k));

        indirectLight = indirectLight + (fr * cosTheta * photon->flux_ * weight);
    }

    // Normalización
    float normalization = (1 - (2.0f / (3.0f * k))) * M_PI * maxDist * maxDist;
    indirectLight = indirectLight / normalization;

    return indirectLight;
}

inline Color gaussianKernel(HitInfo& hit,
                           const vector<const Photon*>& nearestPhotons,
                           double pDiff) 
{
    Color indirectLight(0, 0, 0);

    if (nearestPhotons.empty()) {
        return indirectLight;
    }

    // Calculamos el radio máximo entre los k fotones más cercanos
    float maxDist = 0.0f;

    for (const Photon* photon: nearestPhotons) {
        float dist = (photon->position_ - hit.point).norm();
        maxDist = max(maxDist, dist);
    }

    // BRDF difusa
    Color fr(0, 0, 0);
    fr = hit.kd / (M_PI * pDiff);

    for (const Photon* photon: nearestPhotons) {
        // Dirección del fotón (incidente)
        Direction wi = photon->direction_ * (-1.0);
        
        double cosTheta = max(0.0, hit.normal.dot(wi));
        
        float dist = (photon->position_ - hit.point).norm();
        float weight = ALPHA * (1 - ((1-exp(-BETA*dist*dist/(2*maxDist*maxDist)))/(1-exp(-BETA))));

        indirectLight = indirectLight + (fr * cosTheta * photon->flux_ * weight);
    }

    return indirectLight;
}

inline void cribePhotons(GeometricShape* shape, vector<const Photon*>& photons) {
    for (const Photon* photon: photons) {
        if (!shape->inSurface(photon->position_)) {
            photons.erase(remove(photons.begin(), photons.end(), photon), photons.end());
        }
    }
}

inline Color photonMap(const Ray& ray, 
                       const vector<unique_ptr<GeometricShape>>& shapes,
                       const vector<unique_ptr<PointLight>>& lights,
                       int depth,
                       const PhotonMap& global_map,
                       const PhotonMap& caustic_map,
                       const bool useNEE,
                       const int kernel,
                       const int k_caustic = 100,     // Más fotones para cáusticas
                       const int k_global = 50) 
{
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
    if (useNEE) {
        // Usar Next Event Estimation para iluminación directa
        L = nextEventEstimation(hit, lights, shapes);
    }
    // Si NO usamos NEE, la luz directa viene de los fotones de profundidad 0

    // ============================================
    // 2. ILUMINACIÓN INDIRECTA (Indirect Lighting)
    // ============================================
    double pKill, pDiff, pSpec, pTrans;
    calculateProbabilities(hit, pKill, pDiff, pSpec, pTrans);

    // Solo calculada cuando el material es no delta (tiene valores != 0 en la componente difusa)
    Color indirectLight(0, 0, 0);
    if (isNonDeltaMaterial(hit)) {        
        array<float, 3> queryPoint = pointToArray(hit.point);
        
        // Cáusticas (más fotones, área más pequeña)
        vector<const Photon*> causticPhotons = caustic_map.nearest_neighbors(queryPoint, k_caustic);        
        // Global (menos fotones, área más grande)
        vector<const Photon*> globalPhotons = global_map.nearest_neighbors(queryPoint, k_global);

        Color causticContrib, globalContrib;

        cribePhotons(hit.shape, globalPhotons);
        cribePhotons(hit.shape, causticPhotons);

        switch (kernel) {
            case 1: // Caja
                causticContrib = boxKernel(hit, causticPhotons, pDiff);
                globalContrib = boxKernel(hit, globalPhotons, pDiff);
                indirectLight = causticContrib + globalContrib;
                break;
            case 2: // Triángulo
                causticContrib = triangleKernel(hit, causticPhotons, pDiff);
                globalContrib = triangleKernel(hit, globalPhotons, pDiff);
                indirectLight = causticContrib + globalContrib;
                break;
            case 3: // Gaussiano
                causticContrib = gaussianKernel(hit, causticPhotons, pDiff);
                globalContrib = gaussianKernel(hit, globalPhotons, pDiff);
                indirectLight = causticContrib + globalContrib;
                break;
            default:
                break;
        }
    } else {
        double rrValue = dis0(gen);
        if (rrValue < pSpec) {
            // ===== LÓBULO ESPECULAR (Reflexión perfecta) =====
            Direction wo = ray.d * (-1.0); // Dirección hacia afuera (opuesta al rayo incidente)
            Direction newRayDir = perfectReflection(wo, hit.normal);

            Ray newRay(hit.point, newRayDir);

            indirectLight = indirectLight + 
                           (photonMap(newRay, shapes, lights, depth + 1, global_map, caustic_map, 
                                               useNEE, kernel, k_caustic, k_global) * pSpec);

        } else {
            // ===== LÓBULO DE TRANSMISIÓN (Refracción) =====
            Direction wo = ray.d * (-1.0); // Dirección hacia afuera
            Direction newRayDir;

            // Determinar si estamos entrando o saliendo del objeto
            // Si n·wo > 0, el rayo viene del exterior (entrando)
            // Si n·wo < 0, el rayo viene del interior (saliendo)
            double cosTheta = hit.normal.dot(wo);
            
            // NOTA: Esta implementación asume transiciones aire ↔ material.
            // LIMITACIÓN: No maneja correctamente objetos dieléctricos anidados
            // (ej: esfera de vidrio dentro de otra esfera de vidrio).
            // Para soportar anidamiento, se necesitaría un stack de IORs que 
            // trackee los materiales atravesados en el camino del rayo.
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
            //Color throughput = hit.kt; // Coeficiente de transmisión
            // Refracción exitosa
            newRayDir = wt;
            if (wt.d[0] == 0 && wt.d[1] == 0 && wt.d[2] == 0) {
                // Reflexión interna total - usar reflexión en lugar de refracción
                newRayDir = perfectReflection(wo, effectiveNormal);
                //throughput = Color(0,0,0); // Asumir que toda la energía se refleja
            }

            Ray newRay(hit.point, newRayDir);

            indirectLight = indirectLight + 
                            (photonMap(newRay, shapes, lights, depth + 1, global_map, caustic_map, 
                                               useNEE, kernel, k_caustic, k_global) /*/ pTrans*/);
        }
    }
    
    return L + indirectLight;
}


#endif // PHOTON_MAPPING_HPP