#ifndef COLOR_HPP
#define COLOR_HPP

class Color {
    public:
        double r, g, b;
        Color(double r_ = 0.0, double g_ = 0.0, double b_ = 0.0);

        // Operaciones para colores
        Color operator+(const Color& other) const;
        Color operator-(const Color& other) const;
        Color operator*(double scalar) const;
        Color operator/(double scalar) const;
};

class CoefficientColor {
    public:
        Color kd, ks, kt; // Coeficientes para kd, ks, kt
        CoefficientColor(Color kd_ = Color(), Color ks_ = Color(), Color kt_ = Color());
};

#endif // COLOR_HPP