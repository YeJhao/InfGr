/**
 * File: imaging.cpp
 * Authors: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
 * 
 * Comments: In order to compile, you need to have Eigen library installed.
 *           Install it via:
 *              wget -O Eigen.zip https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip 
 *           Then unzip it and:
 *              a) Include the folder to /usr/local/include
 *              b) Just leave the folder at the same level of the working folder
 * 
 */

#include <stdexcept>

using namespace std;

class PixelRGB {
    public: 
        double R, G, B;

        PixelRGB() : R(0.0), G(0.0), B(0.0) {}
        PixelRGB(double r, double g, double b) : R(r), G(g), B(b) {}

        PixelRGB operator+(const PixelRGB& other) const {
            return PixelRGB(R + other.R, G + other.G, B + other.B);
        }

        PixelRGB operator*(double scalar) const {
            return PixelRGB(R * scalar, G * scalar, B * scalar);
        }
};

class Image {
    public:
        PixelRGB** imagen;
        int width;
        int height;

        // Empty image constructor
        Image(int w, int h) : width(w), height(h) {
            imagen = new PixelRGB*[height]; // Rows
            for (int i = 0; i < height; ++i) {
                imagen[i] = new PixelRGB[width]; // Columns
            }
        }

        // Copy constructor
        Image(const Image& other) : width(other.width), height(other.height) {
            imagen = new PixelRGB*[height];
            for (int i = 0; i < height; ++i) {
                imagen[i] = new PixelRGB[width];
                for (int j = 0; j < width; ++j) {
                    imagen[i][j] = other.imagen[i][j];
                }
            }
        }

        // Destructor
        ~Image() {
            for (int i = 0; i < height; ++i) {
                delete[] imagen[i];
            }
            delete[] imagen;
        }

        // Addition operator
        Image operator+(const Image& other) const {
            if (width != other.width || height != other.height) {
                throw invalid_argument("Images must have the same dimensions for addition.");
            }

            Image result(width, height);
            for (int i = 0; i < height; ++i) {
                for (int j = 0; j < width; ++j) {
                    result.imagen[i][j] = imagen[i][j] + other.imagen[i][j];
                }
            }
            return result;
        }
};

// img should be a HDR image with values in [0.0, +inf)
Image clamping(const Image& img) {
    Image result(img.width, img.height);
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            const PixelRGB& pixel = img.imagen[i][j];
            int r = static_cast<int>(pixel.R / 255.0);
            int g = static_cast<int>(pixel.G / 255.0);
            int b = static_cast<int>(pixel.B / 255.0);

            r = max(0, min(255, r));
            g = max(0, min(255, g));
            b = max(0, min(255, b));
            result.imagen[i][j] = PixelRGB(r, g, b);
        }
    }

    return result;
}

void main(){
    
}