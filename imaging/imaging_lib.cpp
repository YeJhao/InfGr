#include "imaging.hpp"
#include <stdexcept>
#include <iostream>
#include <string>
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <png.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

using namespace std;
using namespace Imf;
using namespace Imath;

// PixelRGB implementations
PixelRGB::PixelRGB() : R(0.0), G(0.0), B(0.0) {}
PixelRGB::PixelRGB(double r, double g, double b) : R(r), G(g), B(b) {}

PixelRGB PixelRGB::operator+(const PixelRGB& other) const {
    return PixelRGB(R + other.R, G + other.G, B + other.B);
}

PixelRGB PixelRGB::operator*(double scalar) const {
    return PixelRGB(R * scalar, G * scalar, B * scalar);
}

// Image implementations
Image::Image(int w, int h) : width(w), height(h) {
    imagen = new PixelRGB*[height]; // Rows
    for (int i = 0; i < height; ++i) {
        imagen[i] = new PixelRGB[width]; // Columns
    }
}

// Copy constructor
Image::Image(const Image& other) : width(other.width), height(other.height) {
    imagen = new PixelRGB*[height];
    for (int i = 0; i < height; ++i) {
        imagen[i] = new PixelRGB[width];
        for (int j = 0; j < width; ++j) {
            imagen[i][j] = other.imagen[i][j];
        }
    }
}

// Destructor
Image::~Image() {
    for (int i = 0; i < height; ++i) {
        delete[] imagen[i];
    }
    delete[] imagen;
}

// Addition operator
Image Image::operator+(const Image& other) const {
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

Image& Image::operator=(const Image& other) {
    if (this != &other) {
        // Free existing memory
        for (int i = 0; i < height; ++i) {
            delete[] imagen[i];
        }
        delete[] imagen;

        // Copy new data
        width = other.width;
        height = other.height;
        imagen = new PixelRGB*[height];
        for (int i = 0; i < height; ++i) {
            imagen[i] = new PixelRGB[width];
            for (int j = 0; j < width; ++j) {
                imagen[i][j] = other.imagen[i][j];
            }
        }
    }
    return *this;
}

void saveHDRImage(const Image& img, const string& filename) {
    // Crea un buffer compatible con OpenEXR
    Array2D<Rgba> pixels;
    pixels.resizeErase(img.height, img.width);

    // Copia los datos de la matriz a la estructura Rgba
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            const PixelRGB& p = img.imagen[i][j];
            pixels[i][j] = Rgba(
                static_cast<half>(p.R),
                static_cast<half>(p.G),
                static_cast<half>(p.B),
                static_cast<half>(1.0f) // alpha = 1.0
            );
        }
    }

    try {
        RgbaOutputFile file(filename.c_str(), img.width, img.height, WRITE_RGBA);
        file.setFrameBuffer(&pixels[0][0], 1, img.width);
        file.writePixels(img.height);
    } catch (const std::exception& e) {
        throw runtime_error("Cannot create HDR: " + string(e.what()));
    }
}

// Function to save LDR image as PNG
void savePNGImage(const Image& img, const string& filename) {
    FILE *file = fopen(filename.c_str(), "wb");
    if (!file) {
        throw runtime_error("Cannot open file for writing: " + filename);
    }
    
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(file);
        throw runtime_error("Cannot create PNG write structure");
    }
    
    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, NULL);
        fclose(file);
        throw runtime_error("Cannot create PNG info structure");
    }
    
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(file);
        throw runtime_error("Error during PNG creation");
    }
    
    png_init_io(png, file);
    png_set_IHDR(png, info, img.width, img.height, 8, PNG_COLOR_TYPE_RGB, 
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    
    png_write_info(png, info);
    
    // Create row pointers
    png_bytep* row_pointers = new png_bytep[img.height];
    for (int i = 0; i < img.height; ++i) {
        row_pointers[i] = new png_byte[img.width * 3];
        for (int j = 0; j < img.width; ++j) {
            // Clamp values to [0, 1] and convert to [0, 255]
            row_pointers[i][j * 3] = static_cast<png_byte>(max(0.0, min(1.0, img.imagen[i][j].R)) * 255);
            row_pointers[i][j * 3 + 1] = static_cast<png_byte>(max(0.0, min(1.0, img.imagen[i][j].G)) * 255);
            row_pointers[i][j * 3 + 2] = static_cast<png_byte>(max(0.0, min(1.0, img.imagen[i][j].B)) * 255);
        }
    }
    
    png_write_image(png, row_pointers);
    png_write_end(png, NULL);
    
    // Clean up
    for (int i = 0; i < img.height; ++i) {
        delete[] row_pointers[i];
    }
    delete[] row_pointers;
    
    png_destroy_write_struct(&png, &info);
    fclose(file);
    
    cout << "LDR image saved successfully as: " << filename << endl;
}

// Additional imaging functions (you can add them as needed)
Image clamping(const Image& img) {
    Image result(img.width, img.height);
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            result.imagen[i][j].R = max(0.0, min(1.0, img.imagen[i][j].R));
            result.imagen[i][j].G = max(0.0, min(1.0, img.imagen[i][j].G));
            result.imagen[i][j].B = max(0.0, min(1.0, img.imagen[i][j].B));
        }
    }
    return result;
}

Image ecualization(const Image& img) {
    double minR = 1e9, maxR = -1e9;
    double minG = 1e9, maxG = -1e9;
    double minB = 1e9, maxB = -1e9;

    // Find min and max for each channel
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            minR = min(minR, img.imagen[i][j].R);
            maxR = max(maxR, img.imagen[i][j].R);
            minG = min(minG, img.imagen[i][j].G);
            maxG = max(maxG, img.imagen[i][j].G);
            minB = min(minB, img.imagen[i][j].B);
            maxB = max(maxB, img.imagen[i][j].B);
        }
    }

    Image result(img.width, img.height);
    // Normalize each channel
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            result.imagen[i][j].R = (maxR > minR) ? (img.imagen[i][j].R - minR) / (maxR - minR) : 0.0;
            result.imagen[i][j].G = (maxG > minG) ? (img.imagen[i][j].G - minG) / (maxG - minG) : 0.0;
            result.imagen[i][j].B = (maxB > minB) ? (img.imagen[i][j].B - minB) / (maxB - minB) : 0.0;
        }
    }
    return result;
}

Image clamp_ecualization(const Image& img, double threshold) {
    double minR = 1e9, maxR = -1e9;
    double minG = 1e9, maxG = -1e9;
    double minB = 1e9, maxB = -1e9;

    // Find min and max for each channel
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            minR = min(minR, img.imagen[i][j].R);
            maxR = max(maxR, img.imagen[i][j].R);
            minG = min(minG, img.imagen[i][j].G);
            maxG = max(maxG, img.imagen[i][j].G);
            minB = min(minB, img.imagen[i][j].B);
            maxB = max(maxB, img.imagen[i][j].B);
        }
    }

    Image result(img.width, img.height);
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            // Clamp and normalize
            double clampedR = min(threshold, img.imagen[i][j].R);
            double clampedG = min(threshold, img.imagen[i][j].G);
            double clampedB = min(threshold, img.imagen[i][j].B);
            
            result.imagen[i][j].R = (threshold > minR) ? (clampedR - minR) / (threshold - minR) : 0.0;
            result.imagen[i][j].G = (threshold > minG) ? (clampedG - minG) / (threshold - minG) : 0.0;
            result.imagen[i][j].B = (threshold > minB) ? (clampedB - minB) / (threshold - minB) : 0.0;
        }
    }
    return result;
}

Image gamma_curve(const Image& img, double gamma) {
    Image result(img.width, img.height);
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            result.imagen[i][j].R = pow(max(0.0, min(1.0, img.imagen[i][j].R)), 1.0 / gamma);
            result.imagen[i][j].G = pow(max(0.0, min(1.0, img.imagen[i][j].G)), 1.0 / gamma);
            result.imagen[i][j].B = pow(max(0.0, min(1.0, img.imagen[i][j].B)), 1.0 / gamma);
        }
    }
    return result;
}

Image clamp_gamma(const Image& img, double clamp_threshold, double gamma) {
    // 1. Clamping
    Image clamped = clamp_ecualization(img, clamp_threshold);
    // 2. Gamma curve
    Image gamma_img = gamma_curve(clamped, gamma);
    return gamma_img;
}

// Function to load HDR image from OpenEXR file
Image loadHDRImage(const string& filename) {
    try {
        RgbaInputFile file(filename.c_str());
        Box2i dw = file.dataWindow();
        
        int width = dw.max.x - dw.min.x + 1;
        int height = dw.max.y - dw.min.y + 1;
        
        Array2D<Rgba> pixels(height, width);
        file.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
        file.readPixels(dw.min.y, dw.max.y);
        
        Image img(width, height);
        
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                img.imagen[i][j].R = pixels[i][j].r;
                img.imagen[i][j].G = pixels[i][j].g;
                img.imagen[i][j].B = pixels[i][j].b;
            }
        }
        
        cout << "HDR image loaded successfully: " << width << "x" << height << " pixels" << endl;
        return img;
        
    } catch (const exception& e) {
        throw runtime_error("Error loading HDR file: " + string(e.what()));
    }
}