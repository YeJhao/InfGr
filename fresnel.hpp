#ifndef FRESNEL_HPP
#define FRESNEL_HPP

#include "geometry.hpp"
#include <cmath>

using namespace std;

/*
 * Pre:  Dado el rayo incidente y la dirección de la normal sobre la
 *       superficie que intersecta "incidente", junto a los coeficientes
 *       de los dos medios correspondientes.
 * Post: Devuelve el coeficiente de Fresnel, que determina la cantidad de
 *       luz que se refleja o refracta sobre una superficie.
 */
inline double fresnel(const Direction& incidente, const Direction& n, 
                      const double eta1, const double eta2) {
    double cosTheta_i = n.dot(incidente);

    // Calcular sinThetaT usando la ley de Snell
    double sinTheta_i = sqrt(1.0 - cosTheta_i * cosTheta_i);
    double sinTheta_t = eta1 / eta2 * sinTheta_i;

    // Manejar la reflexión total interna
    if (sinTheta_t > 1.0) {
        return 1.0;
    }

    double cosTheta_t = sqrt(1.0 - sinTheta_t * sinTheta_t);

    double Rs = (eta1 * cosTheta_i - eta2 * cosTheta_t) / (eta1 * cosTheta_i + eta2 * cosTheta_t);
    double Rp = (eta1 * cosTheta_t - eta2 * cosTheta_i) / (eta1 * cosTheta_t + eta2 * cosTheta_i);

    return (Rs * Rs + Rp * Rp) / 2.0;
}

/*
TODO:
Meterlo en otro bsdf_utils.hpp si es solo esto
Usarlo en el pathtracer con algún parámetro para que utilice fresnel o que lo use siempre
¿Diferenciar plástico y dieléctrico al hacer hit?
 */

#endif // FRESNEL_HPP