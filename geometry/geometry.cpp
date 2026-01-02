/*
* geometry.cpp
* Autores: Jiahao Ye (875490) & Raúl Soler Fernández (875478)
*
* Este fichero contiene la implementación de las clases Point, Direction y Camera,
* que representan puntos, direcciones y cámaras en el espacio 3D.
*/

#include "geometry.hpp"
using namespace std;

// Funciones de creación de la clase Point
Point::Point() : coords(0.0, 0.0, 0.0) {}
Point::Point(double x, double y, double z) : coords(x, y, z) {}
Point::Point(const Vector3d& vec) : coords(vec) {}

// Métodos de acceso a las coordenadas
double Point::x() const { return coords.x(); }
double Point::y() const { return coords.y(); }
double Point::z() const { return coords.z(); }

// Funciones de creación de la clase Direction
Direction::Direction() : d(0.0, 0.0, 0.0) {}
Direction::Direction(double dx, double dy, double dz) : d(dx, dy, dz) {}
Direction::Direction(const Vector3d& vec) : d(vec) {}

// Métodos de acceso a las componentes
double Direction::x() const { return d.x(); }
double Direction::y() const { return d.y(); }
double Direction::z() const { return d.z(); }

// Devuelve la norma de la dirección
double Direction::norm() const {
    return d.norm();
}

// Devuelve la dirección normalizada (sin modificar el objeto original)
Direction Direction::normalized() const {
    return Direction(d.normalized());
}

// Normaliza la dirección (modifica el objeto original)
Direction Direction::normalize() {
    d.normalize();
    return *this;
}

// Operador suma de direcciones
Direction Direction::operator+(const Direction& other) const {
    return Direction(d + other.d);
}

// Operador resta de direcciones
Direction Direction::operator-(const Direction& other) const {
    return Direction(d - other.d);
}

// Operador multiplicación por escalar
Direction Direction::operator*(double scalar) const {
    return Direction(d * scalar);
}

// Operador división por escalar
Direction Direction::operator/(double scalar) const {
    return Direction(d / scalar);
}

// Producto escalar entre direcciones
double Direction::dot(const Direction& other) const {
    return d.dot(other.d);
}

// Producto vectorial entre direcciones
Direction Direction::cross(const Direction& other) const {
    return Direction(d.cross(other.d));
}

// Resta de puntos devuelve una dirección
Direction Point::operator-(const Point& other) const {
    return Direction(coords - other.coords);
}

// Suma de punto y dirección devuelve un punto
Point Point::operator+(const Direction& dir) const {
    return Point(coords + dir.d);
}

// Operador de salida para Point y Direction
std::ostream& operator<<(std::ostream& os, const Point& point) {
    os << "(" << point.x() << ", " << point.y() << ", " << point.z() << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Direction& dir) {
    os << "(" << dir.x() << ", " << dir.y() << ", " << dir.z() << ")";
    return os;
}

// Constructor de Camera (pinhole, sin profundidad de campo)
Camera::Camera(const Point& origin_, const Direction& l_, const Direction& u_, const Direction& f_)
    : origin(origin_), l(l_), u(u_), f(f_), apertureRadius(0.0), focalLength(1.0), focalDistance(1.0) {

    // Construir matriz de transformación de espacio cámara a espacio mundo
    transformation_matrix(0,0) = l.x(); transformation_matrix(0,1) = u.x(); transformation_matrix(0,2) = f.x(); transformation_matrix(0,3) = origin.x();
    transformation_matrix(1,0) = l.y(); transformation_matrix(1,1) = u.y(); transformation_matrix(1,2) = f.y(); transformation_matrix(1,3) = origin.y();
    transformation_matrix(2,0) = l.z(); transformation_matrix(2,1) = u.z(); transformation_matrix(2,2) = f.z(); transformation_matrix(2,3) = origin.z();
    transformation_matrix(3,0) = 0;     transformation_matrix(3,1) = 0;     transformation_matrix(3,2) = 0;     transformation_matrix(3,3) = 1;
    
    inverse_transformation_matrix = transformation_matrix.inverse();
}

// Constructor de Camera con profundidad de campo
Camera::Camera(const Point& origin_, const Direction& l_, const Direction& u_, const Direction& f_,
               double apertureRadius_, double focalDistance_)
    : origin(origin_), l(l_), u(u_), f(f_), apertureRadius(apertureRadius_), focalDistance(focalDistance_) {

    // Calcular focal length usando la ecuación de lente delgada: 1/f = 1/u + 1/v
    // image_dist = 1 (distancia del sensor a la lente, fija)
    // object_dist = focalDistance (distancia del objeto enfocado a la lente)
    double image_dist = 1.0;
    double object_dist = focalDistance_;
    focalLength = 1.0 / (1.0/image_dist + 1.0/object_dist);

    // Construir matriz de transformación de espacio cámara a espacio mundo
    transformation_matrix(0,0) = l.x(); transformation_matrix(0,1) = u.x(); transformation_matrix(0,2) = f.x(); transformation_matrix(0,3) = origin.x();
    transformation_matrix(1,0) = l.y(); transformation_matrix(1,1) = u.y(); transformation_matrix(1,2) = f.y(); transformation_matrix(1,3) = origin.y();
    transformation_matrix(2,0) = l.z(); transformation_matrix(2,1) = u.z(); transformation_matrix(2,2) = f.z(); transformation_matrix(2,3) = origin.z();
    transformation_matrix(3,0) = 0;     transformation_matrix(3,1) = 0;     transformation_matrix(3,2) = 0;     transformation_matrix(3,3) = 1;
    
    inverse_transformation_matrix = transformation_matrix.inverse();
}

// Convierte una dirección del espacio cámara al espacio mundo
Direction Camera::cameraToWorld(const Direction& cameraDir) const {
    Vector3d cameraVec(cameraDir.x(), cameraDir.y(), cameraDir.z());
    Vector3d worldVec = transformation_matrix.block<3,3>(0,0) * cameraVec;
    return Direction(worldVec);
}

// Convierte una dirección del espacio mundo al espacio cámara
Direction Camera::worldToCamera(const Direction& worldDir) const {
    Vector3d worldVec(worldDir.x(), worldDir.y(), worldDir.z());
    Vector3d cameraVec = inverse_transformation_matrix.block<3,3>(0,0) * worldVec;
    return Direction(cameraVec);
}

// Convierte un punto del espacio cámara al espacio mundo
Point Camera::cameraToWorld(const Point& cameraPoint) const {
    Vector4d cameraVec(cameraPoint.x(), cameraPoint.y(), cameraPoint.z(), 1.0);
    Vector4d worldVec = transformation_matrix * cameraVec;
    return Point(worldVec.x(), worldVec.y(), worldVec.z());
}

// Convierte un punto del espacio mundo al espacio cámara
Point Camera::worldToCamera(const Point& worldPoint) const {
    Vector4d worldVec(worldPoint.x(), worldPoint.y(), worldPoint.z(), 1.0);
    Vector4d cameraVec = inverse_transformation_matrix * worldVec;
    return Point(cameraVec.x(), cameraVec.y(), cameraVec.z());
}