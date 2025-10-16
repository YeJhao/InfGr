#ifndef IMAGING_HPP
#define IMAGING_HPP

#include <string>

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

        // Empty image constructor
        Image(int w, int h);

        // Copy constructor
        Image(const Image& other);

        // Destructor
        ~Image();

        // Addition operator
        Image operator+(const Image& other) const;

        Image& operator=(const Image& other);
};

// Function prototypes
Image loadHDRImage(const std::string& filename);
void savePNGImage(const Image& img, const std::string& filename);
Image clamping(const Image& img);
Image ecualization(const Image& img);
Image clamp_ecualization(const Image& img, double threshold);
Image gamma_curve(const Image& img, double gamma);
Image clamp_gamma(const Image& img, double clamp_threshold, double gamma);

#endif // IMAGING_HPP