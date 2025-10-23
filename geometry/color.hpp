#ifndef COLOR_HPP
#define COLOR_HPP

class Color {
    public:
        double r, g, b;
        Color(double r_ = 0.0, double g_ = 0.0, double b_ = 0.0);
};

class CoefficientColor {
    public:
        Color kd, ks, kt; // Coeficientes para kd, ks, kt
        CoefficientColor(Color kd_ = Color(), Color ks_ = Color(), Color kt_ = Color());
};

#endif // COLOR_HPP