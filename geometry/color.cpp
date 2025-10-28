#include "color.hpp"

Color::Color(double r_, double g_, double b_) : r(r_), g(g_), b(b_) {}

Color Color::operator+(const Color& other) const {
    return Color(r + other.r, g + other.g, b + other.b);
}

Color Color::operator-(const Color& other) const {
    return Color(r - other.r, g - other.g, b - other.b);
}

Color Color::operator*(const Color& other) const {
    return Color(r * other.r, g * other.g, b * other.b);
}

Color Color::operator/(const Color& other) const {
    return Color(r / other.r, g / other.g, b / other.b);
}

Color Color::operator+(double scalar) const {
    return Color(r + scalar, g + scalar, b + scalar);
}

Color Color::operator-(double scalar) const {
    return Color(r - scalar, g - scalar, b - scalar);
}

Color Color::operator*(double scalar) const {
    return Color(r * scalar, g * scalar, b * scalar);
}

Color Color::operator/(double scalar) const {
    return Color(r / scalar, g / scalar, b / scalar);
}
