#ifndef COLOR_HPP
#define COLOR_HPP

class Color {
    public:
        double r, g, b;
        Color(double r_ = 0.0, double g_ = 0.0, double b_ = 0.0);
};

class CoefficientColor {
    public:
        double kd, ks, kt; // Coeficientes para kd, ks, kt
        CoefficientColor(double kd_ = 0.0, double ks_ = 0.0, double kt_ = 0.0);
};

#endif // COLOR_HPP