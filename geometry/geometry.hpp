#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <Eigen/Dense>
#include <iostream>
#include <cmath>
#include <stdexcept>

using Eigen::Vector3d;
using Eigen::Vector4d;
using Eigen::Matrix4d;

// Forward declarations
class Direction;

class Point {
    public:
        Vector3d coords;

        Point();
        Point(double x, double y, double z);
        Point(const Vector3d& vec);

        Direction operator-(const Point& other) const;
        Point operator+(const Direction& dir) const;
        
        double x() const;
        double y() const;
        double z() const;
};

class Direction {
    public: 
        Vector3d d;

        Direction();
        Direction(double dx, double dy, double dz);
        Direction(const Vector3d& vec);

        Point operator+(const Point& point) const;
        Direction operator+(const Direction& other) const;
        Direction operator-(const Direction& other) const;
        Direction operator*(double scalar) const;
        Direction operator/(double scalar) const;
        
        double dot(const Direction& other) const;
        Direction cross(const Direction& other) const;
        double norm() const;
        Direction normalized() const;
        Direction normalize();
        
        double x() const;
        double y() const;
        double z() const;
};

// Camera class for flexible viewpoint positioning
class Camera {
    public:
        Point origin;      // Posición de la cámara
        Direction l, u, f; // Vectores base de la cámara (l=left, u=up, f=forward)
        Matrix4d transformation_matrix;
        Matrix4d inverse_transformation_matrix;

        Camera(const Point& origin_, const Direction& l_, const Direction& u_, const Direction& f_);
        
        // Convert direction from camera coordinates to world coordinates
        Direction cameraToWorld(const Direction& cameraDir) const;
        
        // Convert direction from world coordinates to camera coordinates  
        Direction worldToCamera(const Direction& worldDir) const;
        
        // Convert point from camera coordinates to world coordinates
        Point cameraToWorld(const Point& cameraPoint) const;
        
        // Convert point from world coordinates to camera coordinates
        Point worldToCamera(const Point& worldPoint) const;
};

// Global operators
Direction operator*(double scalar, const Direction& dir);
std::ostream& operator<<(std::ostream& os, const Point& point);
std::ostream& operator<<(std::ostream& os, const Direction& dir);

#endif // GEOMETRY_HPP