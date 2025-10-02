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
#include <iostream>
#include <string>
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <png.h>
#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace Imf;
using namespace Imath;

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

        Image& operator=(const Image& other) {
            if (this != &other) {
                // Free existing memory
                for (int i = 0; i < height; ++i) {
                    delete[] imagen[i];
                }
                delete[] imagen;

                // Copy dimensions
                width = other.width;
                height = other.height;

                // Allocate new memory
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
};

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
                const Rgba& pixel = pixels[i][j];
                img.imagen[i][j] = PixelRGB(pixel.r, pixel.g, pixel.b);
            }
        }
        
        cout << "HDR image loaded successfully: " << width << "x" << height << " pixels" << endl;
        return img;
        
    } catch (const exception& e) {
        throw runtime_error("Error loading HDR file: " + string(e.what()));
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
            const PixelRGB& pixel = img.imagen[i][j];
            row_pointers[i][j * 3] = static_cast<png_byte>(pixel.R);
            row_pointers[i][j * 3 + 1] = static_cast<png_byte>(pixel.G);
            row_pointers[i][j * 3 + 2] = static_cast<png_byte>(pixel.B);
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

// img should be a HDR image with values in [0.0, +inf)
Image clamping(const Image& img) {
    Image result(img.width, img.height);
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            const PixelRGB& pixel = img.imagen[i][j];
            // Clamp HDR values to [0, 1] range, then scale to [0, 255]
            double r = max(0.0, min(1.0, pixel.R)) * 255.0;
            double g = max(0.0, min(1.0, pixel.G)) * 255.0;
            double b = max(0.0, min(1.0, pixel.B)) * 255.0;
            
            result.imagen[i][j] = PixelRGB(r, g, b);
        }
    }

    return result;
}

Image ecualization(const Image& img) {
    double minR = 1e9, maxR = -1e9;
    double minG = 1e9, maxG = -1e9;
    double minB = 1e9, maxB = -1e9;

    // Encuentra mínimos y máximos de cada canal
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            const PixelRGB& pixel = img.imagen[i][j];
            minR = std::min(minR, pixel.R);
            maxR = std::max(maxR, pixel.R);
            minG = std::min(minG, pixel.G);
            maxG = std::max(maxG, pixel.G);
            minB = std::min(minB, pixel.B);
            maxB = std::max(maxB, pixel.B);
        }
    }

    Image result(img.width, img.height);
    // Normaliza cada canal
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            const PixelRGB& pixel = img.imagen[i][j];
            double r = (pixel.R - minR) / (maxR - minR) * 255.0;
            double g = (pixel.G - minG) / (maxG - minG) * 255.0;
            double b = (pixel.B - minB) / (maxB - minB) * 255.0;
            // Clamp por seguridad
            r = std::max(0.0, std::min(255.0, r));
            g = std::max(0.0, std::min(255.0, g));
            b = std::max(0.0, std::min(255.0, b));
            result.imagen[i][j] = PixelRGB(r, g, b);
        }
    }
    return result;
}

Image clamp_ecualization(const Image& img, double threshold) {
    double minR = 1e9, maxR = -1e9;
    double minG = 1e9, maxG = -1e9;
    double minB = 1e9, maxB = -1e9;

    // Encuentra mínimos y máximos de cada canal
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            const PixelRGB& pixel = img.imagen[i][j];
            minR = std::min(minR, pixel.R);
            maxR = std::max(maxR, pixel.R);
            minG = std::min(minG, pixel.G);
            maxG = std::max(maxG, pixel.G);
            minB = std::min(minB, pixel.B);
            maxB = std::max(maxB, pixel.B);
        }
    }

    Image result(img.width, img.height);
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            const PixelRGB& pixel = img.imagen[i][j];
            double r, g, b;

            // R
            if (pixel.R <= threshold) {
                r = (pixel.R - minR) / (threshold - minR) * 255.0;
            } else {
                r = 255.0;
            }
            // G
            if (pixel.G <= threshold) {
                g = (pixel.G - minG) / (threshold - minG) * 255.0;
            } else {
                g = 255.0;
            }
            // B
            if (pixel.B <= threshold) {
                b = (pixel.B - minB) / (threshold - minB) * 255.0;
            } else {
                b = 255.0;
            }

            // Clamp por seguridad
            r = std::max(0.0, std::min(255.0, r));
            g = std::max(0.0, std::min(255.0, g));
            b = std::max(0.0, std::min(255.0, b));
            result.imagen[i][j] = PixelRGB(r, g, b);
        }
    }
    return result;
}

// Hay que usar ecualización antes de aplicar la curva gamma
Image gamma_curve(const Image& img, double gamma) {
    Image result(img.width, img.height);
    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            const PixelRGB& pixel = img.imagen[i][j];
            double r = 255.0 * pow(pixel.R / 255.0, 1.0 / gamma);
            double g = 255.0 * pow(pixel.G / 255.0, 1.0 / gamma);
            double b = 255.0 * pow(pixel.B / 255.0, 1.0 / gamma);
            // Clamp por seguridad
            r = std::max(0.0, std::min(255.0, r));
            g = std::max(0.0, std::min(255.0, g));
            b = std::max(0.0, std::min(255.0, b));
            result.imagen[i][j] = PixelRGB(r, g, b);
        }
    }
    return result;
}

Image clamp_gamma(const Image& img, double clamp_threshold, double gamma) {
    // 1. Clamping
    Image clamped = clamp_ecualization(img, clamp_threshold);
    // 2. Curva gamma
    Image gamma_img = gamma_curve(clamped, gamma);
    return gamma_img;
}

int main(){
    try {
        string hdr_filename;
        Image hdr_image(1, 1); // Temporary initialization
        bool image_loaded = false;
        
        cout << "=== HDR to LDR Tone Mapping ===\n";
        
        // Input validation loop for HDR file
        while (!image_loaded) {
            cout << "Ingrese el nombre del archivo HDR (incluya la extension .exr): ";
            cin >> hdr_filename;
            
            // Check if file has .exr extension
            if (hdr_filename.length() < 4 || hdr_filename.substr(hdr_filename.length() - 4) != ".exr") {
                cout << "Error: El archivo debe tener extension .exr\n";
                continue;
            }
            
            try {
                cout << "Cargando imagen HDR: " << hdr_filename << endl;
                hdr_image = loadHDRImage(hdr_filename);
                image_loaded = true;
            } catch (const exception& e) {
                cout << "Error cargando el archivo: " << e.what() << "\n";
                cout << "Por favor, ingrese un archivo HDR válido.\n";
            }
        }
        
        // Algorithm selection menu
        int algorithm_choice;
        bool valid_choice = false;
        
        while (!valid_choice) {
            cout << "\n=== Seleccione el algoritmo de tone mapping ===\n";
            cout << "1. Clamping\n";
            cout << "2. Ecualización\n";
            cout << "3. Clamping + Ecualización\n";
            cout << "4. Curva Gamma\n";
            cout << "5. Clamping + Gamma\n";
            cout << "Ingrese su opción (1-5): ";
            cin >> algorithm_choice;
            
            if (algorithm_choice >= 1 && algorithm_choice <= 5) {
                valid_choice = true;
            } else {
                cout << "Opción inválida. Por favor, seleccione un número entre 1 y 5.\n";
            }
        }
        
        Image ldr_image(1, 1); // Temporary initialization
        string algorithm_name;
        
        // Execute selected algorithm
        switch (algorithm_choice) {
            case 1: {
                cout << "Aplicando tone mapping con algoritmo de clamping..." << endl;
                ldr_image = clamping(hdr_image);
                algorithm_name = "clamping";
                break;
            }
            case 2: {
                cout << "Aplicando tone mapping con algoritmo de ecualización..." << endl;
                ldr_image = ecualization(hdr_image);
                algorithm_name = "ecualization";
                break;
            }
            case 3: {
                double threshold;
                cout << "Ingrese el valor del umbral para clamping + ecualización: ";
                cin >> threshold;
                cout << "Aplicando tone mapping con algoritmo de clamping + ecualización (threshold=" << threshold << ")..." << endl;
                ldr_image = clamp_ecualization(hdr_image, threshold);
                algorithm_name = "clamp_ecualization_" + to_string(threshold);
                break;
            }
            case 4: {
                double gamma;
                cout << "Ingrese el valor de gamma: ";
                cin >> gamma;
                cout << "Aplicando curva gamma (gamma=" << gamma << ")..." << endl;
                // First apply equalization, then gamma curve
                Image equalized = ecualization(hdr_image);
                ldr_image = gamma_curve(equalized, gamma);
                algorithm_name = "gamma";
                break;
            }
            case 5: {
                double threshold, gamma;
                cout << "Ingrese el valor del umbral para clamping: ";
                cin >> threshold;
                cout << "Ingrese el valor de gamma: ";
                cin >> gamma;
                cout << "Aplicando tone mapping con clamping + gamma (threshold=" << threshold << ", gamma=" << gamma << ")..." << endl;
                ldr_image = clamp_gamma(hdr_image, threshold, gamma);
                algorithm_name = "clamp_gamma";
                break;
            }
        }
        
        // Generate output filename
        string output_filename;
        size_t dot_pos = hdr_filename.find_last_of('.');
        if (dot_pos != string::npos) {
            output_filename = hdr_filename.substr(0, dot_pos) + "_" + algorithm_name + ".png";
        } else {
            output_filename = hdr_filename + "_" + algorithm_name + ".png";
        }
        
        cout << "Guardando imagen LDR como: " << output_filename << endl;
        savePNGImage(ldr_image, output_filename);
        
        cout << "\n=== Proceso completado exitosamente ===\n";
        cout << "Archivo HDR: " << hdr_filename << endl;
        cout << "Archivo LDR: " << output_filename << endl;
        cout << "Algoritmo utilizado: " << algorithm_name << endl;
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}