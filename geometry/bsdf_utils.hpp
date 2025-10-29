#ifndef BSDF_UTILS_HPP
#define BSDF_UTILS_HPP

#include "geometry.hpp"
#include <cmath>

/**
 * Calcula la dirección de reflexión perfecta
 * @param wo Dirección de salida (hacia la cámara, normalizada)
 * @param n Normal de la superficie (normalizada)
 * @return Dirección de reflexión (normalizada)
 */
inline Direction perfectReflection(const Direction& wo, const Direction& n) {
    // ωr = 2(n · ωo)n - ωo
    return (n * (2.0 * n.dot(wo)) - wo).normalized();
}

/**
 * Calcula la dirección de refracción perfecta usando la ley de Snell
 * @param wo Dirección de salida (hacia la cámara, normalizada)
 * @param n Normal de la superficie (normalizada)
 * @param eta Índice de refracción relativo (n1/n2)
 * @param refracted Dirección de refracción resultante (si hay refracción)
 * @return true si hay refracción, false si hay reflexión total interna
 */
inline bool perfectRefraction(const Direction& wo, const Direction& n, double eta, Direction& refracted) {
    double cos_theta_i = n.dot(wo);
    double sin2_theta_i = 1.0 - cos_theta_i * cos_theta_i;
    double sin2_theta_t = (eta * eta) * sin2_theta_i;
    
    // Reflexión total interna
    if (sin2_theta_t >= 1.0) {
        return false;
    }
    
    double cos_theta_t = std::sqrt(1.0 - sin2_theta_t);
    
    // ωt = -η·ωo + (η·cos(θi) - cos(θt))·n
    refracted = (wo * (-eta) + n * (eta * cos_theta_i - cos_theta_t)).normalized();
    
    return true;
}

/**
 * Calcula el coeficiente de Fresnel para materiales dieléctricos (aproximación de Schlick)
 * @param cos_theta Coseno del ángulo entre wo y la normal
 * @param eta Índice de refracción relativo (n1/n2)
 * @return Coeficiente de reflexión de Fresnel [0, 1]
 */
inline double fresnelSchlick(double cos_theta, double eta) {
    double r0 = (1.0 - eta) / (1.0 + eta);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * std::pow(1.0 - cos_theta, 5.0);
}

#endif // BSDF_UTILS_HPP
