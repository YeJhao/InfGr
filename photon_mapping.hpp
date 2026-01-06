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
#include <algorithm>

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

    double rrValue = dis0(gen);

    calculateThroughput(hit, ray, pDiff, pDiff + pSpec, rrValue, newRayDir, throughput, gen, dis0);

    // Crear nuevo rayo
    Ray newRay(hit.point, newRayDir);

    if (photon_list.size() >= static_cast<size_t>(maxPhotonsPerLight)) {
        return; // Hemos alcanzado el número máximo de fotones por luz
    }

    // Marcar que rebotó en delta si no es difuso
    bool nextHasBounced = hasBouncedonDelta || (hit.kt.r > 0 || hit.kt.g > 0 || hit.kt.b > 0);

    // El throughput ya incluye la división por la probabilidad (calculado en calculateThroughput)
    // Por lo tanto: nextFlux = flux * (BSDF * cos / pdf) = flux * throughput
    Color nextFlux = throughput * flux;

    // Llamada recursiva
    recursive_trace_photon(depth + 1, newRay, shapes, nextFlux, 
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
        while (photonsAux.size() < photonsPerLight) {
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
                       const vector<const Photon*>& nearestPhotons) {
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

    // BRDF difusa: kd / π
    Color fr = hit.kd / M_PI;

    for (const Photon* photon: nearestPhotons) {
        // Contribución += BRDF * flujo del fotón / área
        // Nota: NO multiplicamos por cos(θ) porque el flujo ya lo incluye
        indirectLight = indirectLight + (fr * photon->flux_ / area);
    }

    return indirectLight;
}

inline Color triangleKernel(HitInfo& hit,
                           const vector<const Photon*>& nearestPhotons) {
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

    // BRDF difusa: kd / π
    Color fr = hit.kd / M_PI;

    for (const Photon* photon: nearestPhotons) {
        float dist = (photon->position_ - hit.point).norm();
        float weight = 1.0f - (dist / (maxDist));

        // Contribución: BRDF * flujo * peso del kernel
        indirectLight = indirectLight + (fr * photon->flux_ * weight);
    }

    // Normalización
    float normalization = (1 - (2.0f / 3.0f)) * M_PI * maxDist * maxDist;
    indirectLight = indirectLight / normalization;

    return indirectLight;
}

inline Color gaussianKernel(HitInfo& hit,
                           const vector<const Photon*>& nearestPhotons) 
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

    // BRDF difusa: kd / π
    Color fr = hit.kd / M_PI;

    // Área del disco (normalización de densidad)
    float area = M_PI * maxDist * maxDist;

    // Calcular suma de pesos para normalización correcta
    float sumWeights = 0.0f;

    for (const Photon* photon: nearestPhotons) {
        float dist = (photon->position_ - hit.point).norm();
        float weight = ALPHA * (1 - ((1-exp(-BETA*dist*dist/(2*maxDist*maxDist)))/(1-exp(-BETA))));
        sumWeights += weight;

        // Contribución: BRDF * flujo * peso del kernel
        indirectLight = indirectLight + (fr * photon->flux_ * weight);
    }

    // Normalizar por área y por suma de pesos (conserva energía)
    if (sumWeights > 1e-6f) {
        indirectLight = indirectLight / (area * sumWeights / nearestPhotons.size());
    }

    return indirectLight;
}

inline void cribePhotons(GeometricShape* shape, vector<const Photon*>& photons) {
    // Usar erase-remove idiom correctamente (sin iterar y modificar a la vez)
    photons.erase(
        remove_if(photons.begin(), photons.end(), 
            [shape](const Photon* photon) {
                return !shape->inSurface(photon->position_);
            }),
        photons.end()
    );
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

    // - Componente difusa: usar mapa de fotones
    // - Componente especular (sin transmisión): seguir rayo reflejado
    // - Componente transmisiva: seguir rayo refractado
    // Para materiales mixtos (plástico), evaluar AMBAS componentes
    
    Color indirectLight(0, 0, 0);
    
    double maxKd = max(hit.kd.r, max(hit.kd.g, hit.kd.b));
    double maxKs = max(hit.ks.r, max(hit.ks.g, hit.ks.b));
    double maxKt = max(hit.kt.r, max(hit.kt.g, hit.kt.b));
    
    // ===== COMPONENTE DIFUSA (si existe) =====
    if (maxKd > 1e-6) {
        array<float, 3> queryPoint = pointToArray(hit.point);
        
        vector<const Photon*> causticPhotons = caustic_map.nearest_neighbors(queryPoint, k_caustic);        
        vector<const Photon*> globalPhotons = global_map.nearest_neighbors(queryPoint, k_global);

        cribePhotons(hit.shape, globalPhotons);
        cribePhotons(hit.shape, causticPhotons);

        Color causticContrib, globalContrib;

        switch (kernel) {
            case 1:
                causticContrib = boxKernel(hit, causticPhotons);
                globalContrib = boxKernel(hit, globalPhotons);
                break;
            case 2:
                causticContrib = triangleKernel(hit, causticPhotons);
                globalContrib = triangleKernel(hit, globalPhotons);
                break;
            case 3:
                causticContrib = gaussianKernel(hit, causticPhotons);
                globalContrib = gaussianKernel(hit, globalPhotons);
                break;
            default:
                break;
        }
        indirectLight = indirectLight + causticContrib + globalContrib;
    }
    
    // ===== COMPONENTE ESPECULAR Y/O TRANSMISIVA =====
    // Según las diapositivas: seguir rayo a través de superficies delta
    // Para dieléctricos (kt dominante): seguir transmisión
    // Para espejos (ks dominante): seguir reflexión
    if (maxKs > 1e-6 && maxKt > 1e-6) {
        // Material con ambos: seguir el camino DOMINANTE
        if (maxKt >= maxKs) {
            // Transmisión dominante (dieléctrico típico)
            Direction wo = ray.d * (-1.0);
            double cosTheta = hit.normal.dot(wo);
            double iorFrom, iorTo;
            Direction effectiveNormal;
            
            if (cosTheta > 0) {
                iorFrom = 1.0;
                iorTo = hit.ior;
                effectiveNormal = hit.normal;
            } else {
                iorFrom = hit.ior;
                iorTo = 1.0;
                effectiveNormal = hit.normal * (-1.0);
            }
            
            Direction wt = perfectRefraction(wo, effectiveNormal, iorFrom, iorTo);
            Direction newRayDir;
            
            if (wt.d[0] == 0 && wt.d[1] == 0 && wt.d[2] == 0) {
                // Reflexión interna total
                newRayDir = perfectReflection(wo, effectiveNormal);
            } else {
                newRayDir = wt;
            }
            
            Ray newRay(hit.point, newRayDir);
            Color transmitLight = photonMap(newRay, shapes, lights, depth + 1, global_map, caustic_map, 
                                            useNEE, kernel, k_caustic, k_global);
            // Sin dividir por probabilidad - seguimos el camino dominante
            indirectLight = indirectLight + transmitLight;
        } else {
            // Reflexión dominante (espejo con algo de kt)
            Direction wo = ray.d * (-1.0);
            Direction newRayDir = perfectReflection(wo, hit.normal);
            Ray newRay(hit.point, newRayDir);
            
            Color specularLight = photonMap(newRay, shapes, lights, depth + 1, global_map, caustic_map, 
                                            useNEE, kernel, k_caustic, k_global);
            indirectLight = indirectLight + specularLight;
        }
    }
    else if (maxKs > 1e-6) {
        // Solo especular (espejo puro o plástico)
        Direction wo = ray.d * (-1.0);
        Direction newRayDir = perfectReflection(wo, hit.normal);
        Ray newRay(hit.point, newRayDir);

        Color specularLight = photonMap(newRay, shapes, lights, depth + 1, global_map, caustic_map, 
                                        useNEE, kernel, k_caustic, k_global);
        indirectLight = indirectLight + (specularLight * hit.ks);
    }
    else if (maxKt > 1e-6) {
        // Solo transmisión
        Direction wo = ray.d * (-1.0);
        double cosTheta = hit.normal.dot(wo);
        double iorFrom, iorTo;
        Direction effectiveNormal;
        
        if (cosTheta > 0) {
            iorFrom = 1.0;
            iorTo = hit.ior;
            effectiveNormal = hit.normal;
        } else {
            iorFrom = hit.ior;
            iorTo = 1.0;
            effectiveNormal = hit.normal * (-1.0);
        }
        
        Direction wt = perfectRefraction(wo, effectiveNormal, iorFrom, iorTo);
        Direction newRayDir;
        
        if (wt.d[0] == 0 && wt.d[1] == 0 && wt.d[2] == 0) {
            newRayDir = perfectReflection(wo, effectiveNormal);
        } else {
            newRayDir = wt;
        }
        
        Ray newRay(hit.point, newRayDir);
        Color transmitLight = photonMap(newRay, shapes, lights, depth + 1, global_map, caustic_map, 
                                        useNEE, kernel, k_caustic, k_global);
        indirectLight = indirectLight + transmitLight;
    }
    
    
    /*{
        // =====================================================
        // MATERIAL DELTA (especular y/o refractivo)
        // =====================================================
        
        Direction wo = ray.d * (-1.0); // Dirección saliente (opuesta al rayo incidente)
        
        // Calcular qué componentes tiene el material
        double maxKs = max(hit.ks.r, max(hit.ks.g, hit.ks.b));
        double maxKt = max(hit.kt.r, max(hit.kt.g, hit.kt.b));
        
        // CASO 1: Solo especular (espejo puro, sin transmisión)
        if (maxKt < 1e-6 && maxKs > 1e-6) {
            Direction reflectDir = perfectReflection(wo, hit.normal);
            Ray reflectRay(hit.point, reflectDir);
            
            indirectLight = photonMap(reflectRay, shapes, lights, depth + 1, 
                                      global_map, caustic_map, useNEE, kernel, 
                                      k_caustic, k_global) * hit.ks;
        }
        // CASO 2: Solo transmisión o transmisión dominante (>90%)
        else if (maxKt > 0.9 * (maxKs + maxKt)) {
            // Determinar si estamos entrando o saliendo del objeto
            // cosTheta > 0 significa que wo apunta hacia el mismo lado que la normal
            // es decir, el rayo viene del exterior (entrando al objeto)
            double cosTheta = hit.normal.dot(wo);
            double iorFrom, iorTo;
            Direction effectiveNormal;
            
            if (cosTheta > 0) {
                // Entrando al objeto: aire -> material
                iorFrom = 1.0;
                iorTo = hit.ior;
                effectiveNormal = hit.normal;
            } else {
                // Saliendo del objeto: material -> aire
                iorFrom = hit.ior;
                iorTo = 1.0;
                effectiveNormal = hit.normal * (-1.0);
                cosTheta = -cosTheta;
            }
            
            Direction wt = perfectRefraction(wo, effectiveNormal, iorFrom, iorTo);
            Direction transmitDir;
            
            if (wt.norm() < 1e-6) {
                // Reflexión interna total - TODA la energía se refleja
                transmitDir = perfectReflection(wo, effectiveNormal);
                // En TIR, la luz se refleja, así que usamos ks (o un valor por defecto)
                indirectLight = photonMap(Ray(hit.point, transmitDir), shapes, lights, depth + 1, 
                                          global_map, caustic_map, useNEE, kernel, 
                                          k_caustic, k_global) * Color(1, 1, 1);
            } else {
                transmitDir = wt;
                Ray transmitRay(hit.point, transmitDir);
                
                // En transmisión, usamos kt (casi 1 para dieléctricos puros)
                indirectLight = photonMap(transmitRay, shapes, lights, depth + 1, 
                                          global_map, caustic_map, useNEE, kernel, 
                                          k_caustic, k_global) * hit.kt;
            }
        }
        // CASO 3: Mezcla significativa de especular y transmisión
        else if (maxKs > 1e-6 && maxKt > 1e-6) {
            double totalDelta = maxKs + maxKt;
            double probSpec = maxKs / totalDelta;
            double rrValue = dis0(gen);
            
            if (rrValue < probSpec) {
                // Reflexión especular
                Direction reflectDir = perfectReflection(wo, hit.normal);
                Ray reflectRay(hit.point, reflectDir);
                
                indirectLight = photonMap(reflectRay, shapes, lights, depth + 1, 
                                          global_map, caustic_map, useNEE, kernel, 
                                          k_caustic, k_global) * hit.ks / probSpec;
            } else {
                // Transmisión
                double cosTheta = hit.normal.dot(wo);
                double iorFrom, iorTo;
                Direction effectiveNormal;
                
                if (cosTheta > 0) {
                    iorFrom = 1.0;
                    iorTo = hit.ior;
                    effectiveNormal = hit.normal;
                } else {
                    iorFrom = hit.ior;
                    iorTo = 1.0;
                    effectiveNormal = hit.normal * (-1.0);
                }
                
                Direction wt = perfectRefraction(wo, effectiveNormal, iorFrom, iorTo);
                Direction transmitDir = (wt.norm() < 1e-6) 
                                        ? perfectReflection(wo, effectiveNormal) : wt;
                
                Ray transmitRay(hit.point, transmitDir);
                
                indirectLight = photonMap(transmitRay, shapes, lights, depth + 1, 
                                          global_map, caustic_map, useNEE, kernel, 
                                          k_caustic, k_global) * hit.kt / (1.0 - probSpec);
            }
        }
    }*/

    return L + indirectLight;
}


#endif // PHOTON_MAPPING_HPP