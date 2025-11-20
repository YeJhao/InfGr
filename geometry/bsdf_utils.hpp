/*
* bsdf_utils.hpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene utilidades para el manejo de BSDFs,
* incluyendo funciones para calcular reflexiones y refracciones perfectas.
*/

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
 * @param wo Dirección de salida (hacia el observador)
 * @param n Normal de la superficie (normalizada, apunta hacia el medio "from")
 * @param iorFrom Índice de refracción del medio de origen
 * @param iorTo Índice de refracción del medio de destino
 * @return Dirección refractada (normalizada), o Direction(0,0,0) si hay reflexión interna total
 * 
 * Ley de Snell: η₁·sin(θ₁) = η₂·sin(θ₂)
 * Fórmula vectorial: wt = -η·wo + (η·cos(θ₁) - cos(θ₂))·n
 */
inline Direction perfectRefraction(const Direction& wo, const Direction& n, double iorFrom, double iorTo) {
    double eta = iorFrom / iorTo;  // η = η₁/η₂
    
    // cos(θ₁) = n · wo (ángulo entre dirección incidente y normal)
    double cosTheta_i = n.dot(wo);
    
    // Calcular sin²(θ₁) = 1 - cos²(θ₁)
    double sin2Theta_i = 1.0 - cosTheta_i * cosTheta_i;
    
    // Calcular sin²(θ₂) usando ley de Snell: sin²(θ₂) = η²·sin²(θ₁)
    double sin2Theta_t = eta * eta * sin2Theta_i;
    
    // Comprobar reflexión interna total: si sin²(θ₂) > 1, no hay refracción
    if (sin2Theta_t > 1.0) {
        // Reflexión interna total - devolver vector nulo como señal
        return Direction(0, 0, 0);
    }
    
    // Calcular cos(θ₂) = √(1 - sin²(θ₂))
    double cosTheta_t = sqrt(1.0 - sin2Theta_t);
    
    // Fórmula vectorial de refracción:
    // wt = -η·wo + (η·cos(θ₁) - cos(θ₂))·n
    Direction wt = (wo * (-eta)) + (n * (eta * cosTheta_i - cosTheta_t));
    
    return wt.normalized();
}

#endif // BSDF_UTILS_HPP
