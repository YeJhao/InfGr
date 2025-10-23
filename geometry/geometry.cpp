/**
 * File: geometry.cpp
 * Authors: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
 */

#include "geometry.hpp"
using namespace std;

// Point implementations
Point::Point() : coords(0.0, 0.0, 0.0) {}
Point::Point(double x, double y, double z) : coords(x, y, z) {}
Point::Point(const Vector3d& vec) : coords(vec) {}

double Point::x() const { return coords.x(); }
double Point::y() const { return coords.y(); }
double Point::z() const { return coords.z(); }

// Direction implementations
Direction::Direction() : d(0.0, 0.0, 0.0) {}
Direction::Direction(double dx, double dy, double dz) : d(dx, dy, dz) {}
Direction::Direction(const Vector3d& vec) : d(vec) {}

double Direction::x() const { return d.x(); }
double Direction::y() const { return d.y(); }
double Direction::z() const { return d.z(); }

double Direction::norm() const {
    return d.norm();
}

Direction Direction::normalized() const {
    return Direction(d.normalized());
}

Direction Direction::normalize() {
    d.normalize();
    return *this;
}

Direction Direction::operator+(const Direction& other) const {
    return Direction(d + other.d);
}

Direction Direction::operator-(const Direction& other) const {
    return Direction(d - other.d);
}

Direction Direction::operator*(double scalar) const {
    return Direction(d * scalar);
}

Direction Direction::operator/(double scalar) const {
    return Direction(d / scalar);
}

double Direction::dot(const Direction& other) const {
    return d.dot(other.d);
}

Direction Direction::cross(const Direction& other) const {
    return Direction(d.cross(other.d));
}

// Point operators implementation
Direction Point::operator-(const Point& other) const {
    return Direction(coords - other.coords);
}

Point Point::operator+(const Direction& dir) const {
    return Point(coords + dir.d);
}

std::ostream& operator<<(std::ostream& os, const Point& point) {
    os << "(" << point.x() << ", " << point.y() << ", " << point.z() << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Direction& dir) {
    os << "(" << dir.x() << ", " << dir.y() << ", " << dir.z() << ")";
    return os;
}

Direction operator*(double scalar, const Direction& dir) {
    return dir * scalar;
}

// Camera implementations
Camera::Camera(const Point& origin_, const Direction& l_, const Direction& u_, const Direction& f_)
    : origin(origin_), l(l_), u(u_), f(f_) {

    // Build transformation matrix from camera space to world space
    transformation_matrix(0,0) = l.x(); transformation_matrix(0,1) = u.x(); transformation_matrix(0,2) = f.x(); transformation_matrix(0,3) = origin.x();
    transformation_matrix(1,0) = l.y(); transformation_matrix(1,1) = u.y(); transformation_matrix(1,2) = f.y(); transformation_matrix(1,3) = origin.y();
    transformation_matrix(2,0) = l.z(); transformation_matrix(2,1) = u.z(); transformation_matrix(2,2) = f.z(); transformation_matrix(2,3) = origin.z();
    transformation_matrix(3,0) = 0;     transformation_matrix(3,1) = 0;     transformation_matrix(3,2) = 0;     transformation_matrix(3,3) = 1;
    
    inverse_transformation_matrix = transformation_matrix.inverse();
}

Direction Camera::cameraToWorld(const Direction& cameraDir) const {
    Vector3d cameraVec(cameraDir.x(), cameraDir.y(), cameraDir.z());
    Vector3d worldVec = transformation_matrix.block<3,3>(0,0) * cameraVec;
    return Direction(worldVec);
}

Direction Camera::worldToCamera(const Direction& worldDir) const {
    Vector3d worldVec(worldDir.x(), worldDir.y(), worldDir.z());
    Vector3d cameraVec = inverse_transformation_matrix.block<3,3>(0,0) * worldVec;
    return Direction(cameraVec);
}

Point Camera::cameraToWorld(const Point& cameraPoint) const {
    Vector4d cameraVec(cameraPoint.x(), cameraPoint.y(), cameraPoint.z(), 1.0);
    Vector4d worldVec = transformation_matrix * cameraVec;
    return Point(worldVec.x(), worldVec.y(), worldVec.z());
}

Point Camera::worldToCamera(const Point& worldPoint) const {
    Vector4d worldVec(worldPoint.x(), worldPoint.y(), worldPoint.z(), 1.0);
    Vector4d cameraVec = inverse_transformation_matrix * worldVec;
    return Point(cameraVec.x(), cameraVec.y(), cameraVec.z());
}