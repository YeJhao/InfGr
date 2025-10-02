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

class Planet {
    public:
        // Key points
        Point center;
        Point city_ref;
        // Planet radius
        double radius;
        // Main directions
        Direction axis; // from south to north pole (k)
        Direction azimut0; // in the same plane as equatorial and perpendicular to axis (i)
        Direction equatorial; // perpendicular to axis and azimut0 (j)
        // Transformation matrices from world coordinates to planet coordinates and vice versa
        Matrix4d transformation_matrix;
        Matrix4d inverse_transformation_matrix;

        Planet(const Point& center_, const Direction& axis_, const Point& city_ref_)
            : center(center_), axis(axis_), city_ref(city_ref_) 
        {

            // Check radius consistency
            double radius_from_axis = axis.norm() / 2.0;
            double radius_from_city = (city_ref - center).norm();

            if (abs(radius_from_axis - radius_from_city) > 1e-6) {
                throw invalid_argument("Radio inconsistente. Radio del eje: " 
                    + to_string(radius_from_axis) 
                    + ", Radio desde ciudad: " 
                    + to_string(radius_from_city));
            }

            radius = radius_from_axis;
            // Calculate the direction of center->city
            Direction center_to_city = (city_ref - center);
            // Get the perpendicular vector of the axis and the center->city
            Direction equatorial = axis.cross(center_to_city);
            // Calculate the perpendicular projection of the equatorial and the axis
            azimut0 = equatorial.cross(axis);

            // Normalize directions
            Direction equatorial_norm = equatorial.normalize();
            Direction azimut0_norm = azimut0.normalize();
            Direction axis_norm = axis.normalize();

            // Build transformation matrix
            transformation_matrix(0,0) = azimut0_norm.x();
            transformation_matrix(0,1) = equatorial_norm.x();
            transformation_matrix(0,2) = axis_norm.x();
            transformation_matrix(0,3) = center.x();

            transformation_matrix(1,0) = azimut0_norm.y();
            transformation_matrix(1,1) = equatorial_norm.y();
            transformation_matrix(1,2) = axis_norm.y();
            transformation_matrix(1,3) = center.y();

            transformation_matrix(2,0) = azimut0_norm.z();
            transformation_matrix(2,1) = equatorial_norm.z();
            transformation_matrix(2,2) = axis_norm.z();
            transformation_matrix(2,3) = center.z();

            transformation_matrix(3,0) = 0;
            transformation_matrix(3,1) = 0;
            transformation_matrix(3,2) = 0;
            transformation_matrix(3,3) = 1;

            inverse_transformation_matrix = transformation_matrix.inverse();
        }
};

class Station {
    public:
        double inclination;
        double azimut;
        Planet planet;
        Point position;
        Direction i, j ,k;
        // Transformation matrix from planet coordinates to station coordinates and vice versa
        Matrix4d transformation_matrix;
        Matrix4d inverse_transformation_matrix;

        Station(const Planet& planet_, double inclination_, double azimut_)
            : planet(planet_), inclination(inclination_), azimut(azimut_), position(0, 0, 0) 
        {
            position = Point(
                planet.center.x() + planet.radius * sin(inclination) * cos(azimut),
                planet.center.y() + planet.radius * sin(inclination) * sin(azimut),
                planet.center.z() + planet.radius * cos(inclination)
            );

            cout << "Posicion de la estacion: (" 
                 << position.x() << ", "
                 << position.y() << ", "
                 << position.z() << ")" << endl;

            // k: Normal to surface
            k = (position - planet.center);
            
            // i: Tangent to longitude (azimut direction) 
            i = planet.axis.cross(k);
            
            // j: Tangent to latitude (inclination direction)
            j = i.cross(k);

            Direction i_norm = i.normalize();
            Direction j_norm = j.normalize();
            Direction k_norm = k.normalize();

            transformation_matrix(0,0) = i_norm.x();
            transformation_matrix(0,1) = j_norm.x();
            transformation_matrix(0,2) = k_norm.x();
            transformation_matrix(0,3) = position.x();

            transformation_matrix(1,0) = i_norm.y();
            transformation_matrix(1,1) = j_norm.y();
            transformation_matrix(1,2) = k_norm.y();
            transformation_matrix(1,3) = position.y();

            transformation_matrix(2,0) = i_norm.z();
            transformation_matrix(2,1) = j_norm.z();
            transformation_matrix(2,2) = k_norm.z();
            transformation_matrix(2,3) = position.z();

            transformation_matrix(3,0) = 0;
            transformation_matrix(3,1) = 0;
            transformation_matrix(3,2) = 0;
            transformation_matrix(3,3) = 1;

            inverse_transformation_matrix = transformation_matrix.inverse();
        }

        // Convert direction from local coordinates (i,j,k) to global UCS coordinates
        Direction localToGlobal(const Direction& localDir) const {
            Eigen::Matrix3d rotationMatrix;
            rotationMatrix << inverse_transformation_matrix(0,0), inverse_transformation_matrix(0,1), inverse_transformation_matrix(0,2),
                            inverse_transformation_matrix(1,0), inverse_transformation_matrix(1,1), inverse_transformation_matrix(1,2),
                            inverse_transformation_matrix(2,0), inverse_transformation_matrix(2,1), inverse_transformation_matrix(2,2);
            
            Vector3d localVec(localDir.x(), localDir.y(), localDir.z());
            Vector3d globalVec = rotationMatrix * localVec;
            
            return Direction(globalVec);
        }

        // Convert direction from global UCS coordinates to local coordinates (i,j,k)
        Direction globalToLocal(const Direction& globalDir) const {
            Eigen::Matrix3d rotationMatrix;
            rotationMatrix << transformation_matrix(0,0), transformation_matrix(0,1), transformation_matrix(0,2),
                            transformation_matrix(1,0), transformation_matrix(1,1), transformation_matrix(1,2),
                            transformation_matrix(2,0), transformation_matrix(2,1), transformation_matrix(2,2);
            
            Vector3d globalVec(globalDir.x(), globalDir.y(), globalDir.z());
            Vector3d localVec = rotationMatrix.transpose() * globalVec;
            
            return Direction(localVec);
        }
};

class Connection {
    public:
        Station* launch_station;
        Station* receive_station;
        Direction global_direction;
        Direction launch_local_direction;
        Direction receive_local_direction;
        bool safe_launch;
        bool safe_receive;

        Connection(Station* launch, Station* receive) 
            : launch_station(launch), receive_station(receive) {
            
            // Calculate global direction from launch to receive
            global_direction = (receive_station->position - launch_station->position);
            
            // Convert to local coordinates for each station
            launch_local_direction = launch_station->globalToLocal(global_direction);
            
            // For receive station, we need the INCOMING direction (opposite)
            receive_local_direction = receive_station->globalToLocal(global_direction);
            
            // Check safety: direction must point away from planet surface (positive k component)
            safe_launch = launch_local_direction.z() > 0;   // pointing outward from planet
            safe_receive = receive_local_direction.z() < 0; // incoming from outward
        }

        void printConnection() const {
            cout << "=== CONEXIÓN INTERPLANETARIA ===" << endl;
            cout << "Direccion global UCS: (" 
                 << global_direction.x() << ", "
                 << global_direction.y() << ", "
                 << global_direction.z() << ")" << endl;
            
            cout << "\nEstacion de lanzamiento:" << endl;
            cout << "Coordenadas locales de salida: ("
                 << launch_local_direction.x() << ", "
                 << launch_local_direction.y() << ", "
                 << launch_local_direction.z() << ")" << endl;
            
            cout << "Estacion receptora:" << endl;
            cout << "Coordenadas locales de entrada: ("
                 << receive_local_direction.x() << ", "
                 << receive_local_direction.y() << ", "
                 << receive_local_direction.z() << ")" << endl;
            
            // Safety warnings
            if (!safe_launch) {
                cout << "\nADVERTENCIA: La catapulta cuantica apunta hacia el interior del planeta de lanzamiento. OPERACION PELIGROSA." << endl;
            }
            if (!safe_receive) {
                cout << "\nADVERTENCIA: La materia traspasa el interior del planeta receptor. OPERACION PELIGROSA." << endl;
            }
            if (safe_launch && safe_receive) {
                cout << "\nConexion segura establecida." << endl;
            }
        }
};