/*
* color.cpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la implementación de la clase Color,
* que representa colores RGB y operaciones básicas con ellos.
*/

#include "color.hpp"

// Constructor
Color::Color(double r_, double g_, double b_) : r(r_), g(g_), b(b_) {}

// Suma de colores
Color Color::operator+(const Color& other) const {
    return Color(r + other.r, g + other.g, b + other.b);
}

// Resta de colores
Color Color::operator-(const Color& other) const {
    return Color(r - other.r, g - other.g, b - other.b);
}

// Multiplicación de colores
Color Color::operator*(const Color& other) const {
    return Color(r * other.r, g * other.g, b * other.b);
}

// División de colores
Color Color::operator/(const Color& other) const {
    return Color(r / other.r, g / other.g, b / other.b);
}

// Suma con escalar
Color Color::operator+(double scalar) const {
    return Color(r + scalar, g + scalar, b + scalar);
}

// Resta con escalar
Color Color::operator-(double scalar) const {
    return Color(r - scalar, g - scalar, b - scalar);
}

// Multiplicación con escalar
Color Color::operator*(double scalar) const {
    return Color(r * scalar, g * scalar, b * scalar);
}

// División con escalar
Color Color::operator/(double scalar) const {
    return Color(r / scalar, g / scalar, b / scalar);
}
