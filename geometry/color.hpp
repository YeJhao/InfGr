/*
* color.hpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la definición de la clase Color,
* que representa colores RGB y operaciones básicas con ellos.
*/

#ifndef COLOR_HPP
#define COLOR_HPP

class Color {
    public:
        double r, g, b;
        Color(double r_ = 0.0, double g_ = 0.0, double b_ = 0.0);

        // Operaciones para colores
        Color operator+(const Color& other) const;
        Color operator-(const Color& other) const;
        Color operator*(const Color& other) const;
        Color operator/(const Color& other) const;
        
        Color operator+(double scalar) const;
        Color operator-(double scalar) const;
        Color operator*(double scalar) const;
        Color operator/(double scalar) const;
};

#endif // COLOR_HPP