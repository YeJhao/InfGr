/*
* geometry.hpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la definición de las clases Point, Direction y Camera,
* que representan puntos, direcciones y cámaras en el espacio 3D.
*/

#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <Eigen/Dense>
#include <iostream>
#include <cmath>
#include <stdexcept>

using Eigen::Vector3d;
using Eigen::Vector4d;
using Eigen::Matrix4d;

// Declaración adelantada
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

class Camera {
    public:
        Point origin;      // Posición de la cámara
        Direction l, u, f; // Vectores base de la cámara (l=left, u=up, f=forward)
        Matrix4d transformation_matrix;
        Matrix4d inverse_transformation_matrix;

        Camera(const Point& origin_, const Direction& l_, const Direction& u_, const Direction& f_);
        
        Direction cameraToWorld(const Direction& cameraDir) const;
        Direction worldToCamera(const Direction& worldDir) const;
        Point cameraToWorld(const Point& cameraPoint) const;
        Point worldToCamera(const Point& worldPoint) const;
};

std::ostream& operator<<(std::ostream& os, const Point& point);
std::ostream& operator<<(std::ostream& os, const Direction& dir);

#endif // GEOMETRY_HPP