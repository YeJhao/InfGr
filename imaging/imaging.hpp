/*
* imaging.hpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la definición de la clase Image y PixelRGB,
* además de las funciones para cargar y guardar imágenes HDR y LDR y de 
* procesamiento de Imágenes con tone mapping
*/

#ifndef IMAGING_HPP
#define IMAGING_HPP

#include <string>

using namespace std;

class PixelRGB {
    public: 
        double R, G, B;

        PixelRGB();
        PixelRGB(double r, double g, double b);

        PixelRGB operator+(const PixelRGB& other) const;
        PixelRGB operator*(double scalar) const;
};

class Image {
    public:
        PixelRGB** imagen;
        int width;
        int height;

        // Constructor de imagen vacía
        Image(int w, int h);

        // CConstructor de copia
        Image(const Image& other);

        // Destructor
        ~Image();

        // Operador de suma
        Image operator+(const Image& other) const;

        // Operador de asignación
        Image& operator=(const Image& other);
};

// Funciones de imagen HDR
Image loadHDRImage(const string& filename);
void saveHDRImage(const Image& img, const string& filename);
void savePNGImage(const Image& img, const string& filename);
Image clamping(const Image& img);
Image ecualization(const Image& img);
Image clamp_ecualization(const Image& img, double threshold);
Image gamma_curve(const Image& img, double gamma);
Image clamp_gamma(const Image& img, double clamp_threshold, double gamma);

#endif // IMAGING_HPP