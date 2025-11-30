#ifndef PATH_TRACING_HPP
#define PATH_TRACING_HPP

#include "ray/ray.hpp"
#include "geometry/geometric_shape.hpp"
#include "geometry/sphere.hpp"
#include "geometry/plane.hpp"
#include "geometry/triangle.hpp"
#include "geometry/bsdf_utils.hpp"
#include "geometry/color.hpp"
#include "light/point_light.hpp"
#include <vector>
#include <memory>
#include <random>
#include <limits>
#include <cmath>

using namespace std;

// Variables de configuración de Path Tracing
#define MAX_BOUNCES 100     // Límite de seguridad, Russian Roulette terminará antes (mucho antes)
#define RR_MIN_DEPTH 2      // Profundidad mínima antes de aplicar Russian Roulette

// Estructura para almacenar información de intersección
struct HitInfo {
    bool hit;
    Point point;
    Direction normal;
    Color emission;
    Color kd;
    Color ks;
    Color kt;
    double ior;
    GeometricShape* shape;
    
    // Constructor 
    HitInfo() : hit(false), shape(nullptr) {}
};

/**
 * Encuentra la intersección más cercana entre un rayo y la escena
 */
inline HitInfo findClosestIntersection(const Ray& ray, 
                    const vector<unique_ptr<GeometricShape>>& shapes, 
                    const Point& rayOrigin) {
    HitInfo hitInfo;
    double minDistance = numeric_limits<double>::max();
    
    for (const auto& shape : shapes) {
        vector<Point> intersections = ray.intersections(*shape);
        
        for (const Point& intersection : intersections) {
            Direction toIntersection = intersection - rayOrigin;
            double distance = toIntersection.norm();
            
            if (distance < minDistance && distance > 1e-4) { // Epsilon para evitar auto-intersección
                minDistance = distance;
                hitInfo.hit = true;
                hitInfo.point = intersection;
                hitInfo.shape = shape.get();
                
                // Extraer propiedades del material usando dynamic_cast
                if (auto sphere = dynamic_cast<const Sphere*>(shape.get())) {
                    hitInfo.emission = sphere->emission;
                    hitInfo.kd = sphere->kd;
                    hitInfo.ks = sphere->ks;
                    hitInfo.kt = sphere->kt;
                    hitInfo.ior = sphere->ior;
                    hitInfo.normal = sphere->calculateNormalAtPoint(intersection);
                } else if (auto plane = dynamic_cast<const Plane*>(shape.get())) {
                    hitInfo.emission = plane->emission;
                    hitInfo.kd = plane->kd;
                    hitInfo.ks = plane->ks;
                    hitInfo.kt = plane->kt;
                    hitInfo.ior = plane->ior;
                    hitInfo.normal = plane->normal;
                } else if (auto triangle = dynamic_cast<const Triangle*>(shape.get())) {
                    hitInfo.emission = triangle->emission;
                    hitInfo.kd = triangle->kd;
                    hitInfo.ks = triangle->ks;
                    hitInfo.kt = triangle->kt;
                    hitInfo.ior = triangle->ior;
                    hitInfo.normal = triangle->normal;
                }
            }
        }
    }
    
    return hitInfo;
}

/**
 * Comprueba si hay visibilidad entre dos puntos (para rayos de sombra)
 * @param from Punto de origen
 * @param to Punto destino 
 * @param shapes Geometrías de la escena
 * @return true si no hay obstrucciones, false si hay algo entre ambos puntos
 */
inline bool isVisible(const Point& from, const Point& to, const vector<unique_ptr<GeometricShape>>& shapes) {
    Direction dir = to - from;
    double maxDistance = dir.norm();
    dir = dir / maxDistance; // Normalizar
    
    Ray shadowRay(from, dir);
    
    for (const auto& shape : shapes) {
        vector<Point> intersections = shadowRay.intersections(*shape);
        for (const Point& intersection : intersections) {
            double distance = (intersection - from).norm();
            // Si hay algo entre el punto y la luz (con un pequeño margen)
            if (distance > 1e-4 && distance < maxDistance - 1e-4) {
                return false; // Bloqueado
            }
        }
    }
    
    return true; // Visible
}

/**
 * Genera una dirección aleatoria en el hemisferio según distribución de coseno
 * Usado para muestreo de BRDF difusa
 */
inline Direction cosineSampleHemisphere(mt19937& gen, uniform_real_distribution<double>& dis) {
    // Generación de números aleatorios
    double u1 = dis(gen);
    double u2 = dis(gen);

    // r = sen(θᵢ) = sqrt(u1)
    double r = sqrt(u1);
    double phi = 2.0 * M_PI * u2;
    
    double x = r * cos(phi);
    double y = r * sin(phi);
    double z = sqrt(1.0 - u1);
    
    return Direction(x, y, z);
}

/**
 * Construye una base ortonormal a partir de un vector normal
 */
inline void buildOrthonormalBasis(const Direction& n, Direction& tangent, Direction& bitangent) {
    // Elegir un vector que no sea paralelo a n
    Direction up = (fabs(n.d[0]) > 0.9) ? Direction(0, 1, 0) : Direction(1, 0, 0);
    tangent = n.cross(up).normalized();
    bitangent = n.cross(tangent).normalized();
}

/**
 * Transforma una dirección del espacio local (respecto a la normal) al espacio mundial
 */
inline Direction localToWorld(const Direction& localDir, const Direction& normal, 
                              const Direction& tangent, const Direction& bitangent) {
    return (tangent * localDir.d[0] + 
            bitangent * localDir.d[1] + 
            normal * localDir.d[2]).normalized();
}

/**
 * Calcula las probabilidades de cada lóbulo de la BSDF y la probabilidad de terminar el camino
 */
inline void calculateProbabilities(const HitInfo& hit, double& pKill, double& pDiff, double& pSpec, double& pTrans) {
    double maxKd = max(hit.kd.r, max(hit.kd.g, hit.kd.b)); 
    double maxKs = max(hit.ks.r, max(hit.ks.g, hit.ks.b));
    double maxKt = max(hit.kt.r, max(hit.kt.g, hit.kt.b));
    double s = max(1.0, (maxKd + maxKs + maxKt + 0.1));
    
    // Probabilidad de matar el fotón
    double pdiff = (maxKd / s);
    double pspec = (maxKs / s);
    double ptrans = (maxKt / s);
    pKill = 1.0 - pdiff - pspec - ptrans;

    // Normalización por si no matamos el camino del fotón
    double pContinue = 1.0 - pKill;
    pDiff = pdiff / pContinue;
    pSpec = pspec / pContinue;
    pTrans = ptrans / pContinue;
}

/**
 * Calcula la nueva dirección del rayo y el throughput según el lóbulo seleccionado
 */
inline void calculateThroughput(const HitInfo& hit, const Ray& ray, const double pDiff, const double pSpec,
                        const double rrValue, Direction& newRayDir, Color& throughput, 
                        mt19937& gen, uniform_real_distribution<double>& dis) {
    if (rrValue < pDiff) {
        // ===== LÓBULO DIFUSO =====
        // Muestreo con distribución de coseno
        Direction tangent, bitangent;
        buildOrthonormalBasis(hit.normal, tangent, bitangent);
        Direction localDir = cosineSampleHemisphere(gen, dis);
        newRayDir = localToWorld(localDir, hit.normal, tangent, bitangent);

        // BRDF difusa: kd / π
        // PDF del muestreo de coseno: cos(θ) / π
        // throughput = BRDF * cos(θ) / PDF = kd / π * cos(θ) / (cos(θ) / π) = kd
        throughput = hit.kd / pDiff;

    } else if (rrValue < pSpec) {
        // ===== LÓBULO ESPECULAR (Reflexión perfecta) =====
        Direction wo = ray.d * (-1.0); // Dirección hacia afuera (opuesta al rayo incidente)
        newRayDir = perfectReflection(wo, hit.normal);

        throughput = hit.ks / pSpec;
       
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

        throughput = hit.kt / (1.0 - pDiff - pSpec);
    }
}

/**
 * Calcula la radiancia entrante mediante path tracing recursivo
 * @param ray Rayo actual
 * @param shapes Geometrías de la escena
 * @param lights Luces de la escena
 * @param gen Generador de números aleatorios
 * @param dis Distribución uniforme [0,1]
 * @param depth Profundidad actual de recursión
 * @return Color/radiancia acumulada
 */
inline Color pathTrace(const Ray& ray, 
                       const vector<unique_ptr<GeometricShape>>& shapes,
                       const vector<unique_ptr<PointLight>>& lights,
                       mt19937& gen,
                       uniform_real_distribution<double>& dis,
                       int depth) {   
    // Límite de seguridad (por si acaso)
    if (depth >= MAX_BOUNCES) {
        return Color(0, 0, 0);
    }
    
    // Encontrar intersección más cercana
    HitInfo hit = findClosestIntersection(ray, shapes, ray.o);
    
    // Si no hay intersección, devolver color de fondo (negro)
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
    
    // Calcular probabilidades de cada lóbulo de la BSDF
    double pKill, pDiff, pSpec, pTrans;
    calculateProbabilities(hit, pKill, pDiff, pSpec, pTrans);

    // ============================================
    // RUSSIAN ROULETTE - Terminación
    // ============================================
    double rrValue = dis(gen);
    if (depth >= RR_MIN_DEPTH) {
        if (rrValue < pKill){
            return L; // Terminar el path aquí
        }
    }
    
    // Si no terminamos, procedemos a seleccionar el lóbulo y calcular el nuevo rayo
    // Generar nuevo valor aleatorio para seleccionar el lóbulo
    rrValue = dis(gen);
        
    Direction newRayDir;
    Color throughput; // Factor de transmisión de luz

    calculateThroughput(hit, ray, pDiff, pDiff + pSpec, rrValue, newRayDir, throughput, gen, dis);
    
    // Crear nuevo rayo
    Ray newRay(hit.point, newRayDir);
    
    // Trazar recursivamente
    Color Li = pathTrace(newRay, shapes, lights, gen, dis, depth + 1);
    
    // Acumular iluminación indirecta
    L = L + (throughput * Li);
    
    return L;
}

#endif // PATH_TRACING_HPP
