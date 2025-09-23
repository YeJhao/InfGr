/**
 * File: geometry.cpp
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

#include <Eigen/Dense>
#include <iostream>
#include <cmath>
#include <stdexcept>

using Eigen::Vector3d;
using namespace std;

class Direction;

class Point {
    public:
        Vector3d coords;

        Point(double x, double y, double z) : coords(x, y, z) {}
        Point(const Vector3d& vec) : coords(vec) {}

        Direction operator-(const Point& other) const;
        Point operator+(const Direction& dir) const;
        
        double x() const { return coords.x(); }
        double y() const { return coords.y(); }
        double z() const { return coords.z(); }
};

class Direction {
    public: 
        Vector3d d;

        Direction() : d(0.0, 0.0, 0.0) {}
        Direction(double dx, double dy, double dz) : d(dx, dy, dz) {}
        Direction(const Vector3d& vec) : d(vec) {}

        double mod() const {
            return d.norm();
        }

        Direction normalize() const {
            return Direction(d.normalized());
        }
        
        Direction operator+(const Direction& other) const {
            return Direction(d + other.d);
        }
        
        Direction operator-(const Direction& other) const {
            return Direction(d - other.d);
        }
        
        Direction operator*(double scalar) const {
            return Direction(d * scalar);
        }
        
        Direction operator-() const {
            return Direction(-d);
        }
        
        double dot(const Direction& other) const {
            return d.dot(other.d);
        }
        
        Direction cross(const Direction& other) const {
            return Direction(d.cross(other.d));
        }
        
        double x() const { return d.x(); }
        double y() const { return d.y(); }
        double z() const { return d.z(); }
};

Direction Point::operator-(const Point& other) const {
    return Direction(coords - other.coords);
}

Point Point::operator+(const Direction& dir) const {
    return Point(coords + dir.d);
}

Direction operator*(double scalar, const Direction& dir) {
    return dir * scalar;
}

class Planet {
    public:
        Point center;
        Direction axis;
        Point city_ref;
        double radius;
        Direction azimut0;

        Planet(const Point& center_, const Direction& axis_, const Point& city_ref_)
            : center(center_), axis(axis_), city_ref(city_ref_) 
        {

            double radius_from_axis = axis.mod() / 2.0;
            double radius_from_city = (city_ref - center).mod();

            if (abs(radius_from_axis - radius_from_city) > 1e-6) {
                throw invalid_argument("Radio inconsistente.");
            }

            radius = radius_from_axis;
            axis = axis_.normalize();

            // Calculate the direction of center->city
            Direction center_to_city = (city_ref - center).normalize();
            // Get the perpendicular vector of the previous vector and the axis
            Direction equatorial = axis.cross(center_to_city);
            // Calculate the perpendicular projection of the axis and the equatioral
            azimut0 = axis.cross(equatorial).normalize();
        }
};

class Station {
    public:
        double inclination;
        double azimut;
        Planet planet;
        Point position;
        Direction k, i, j;

        Station(const Planet& planet_, double inclination_, double azimut_)
            : planet(planet_), inclination(inclination_), azimut(azimut_), position(0, 0, 0) {
            
            Direction axis_normalized = planet.axis.normalize();
            Direction azimut0_normalized = planet.azimut0.normalize();
            Direction equatorial_perpendicular = axis_normalized.cross(azimut0_normalized).normalize();
            
            Direction surface_direction = 
                azimut0_normalized * (sin(inclination) * cos(azimut)) +
                equatorial_perpendicular * (sin(inclination) * sin(azimut)) +
                axis_normalized * cos(inclination);
                
            position = planet.center + surface_direction * planet.radius;

            // k: Normal to surface
            k = surface_direction; 
            
            // i: Tangent to longitude (azimut direction) 
            i = azimut0_normalized * (-sin(azimut)) + equatorial_perpendicular * cos(azimut);
            i = i * cos(inclination);
            i = i.normalize();
            
            // j: Tangent to latitude (inclination direction)
            j = k.cross(i).normalize();
        }

        // Convert direction from local coordinates (i,j,k) to global UCS coordinates
        Direction localToGlobal(const Direction& localDir) const {
            return i * localDir.x() + j * localDir.y() + k * localDir.z();
        }

        // Convert direction from global UCS coordinates to local coordinates (i,j,k)
        Direction globalToLocal(const Direction& globalDir) const {
            return Direction(
                globalDir.dot(i),
                globalDir.dot(j),
                globalDir.dot(k)
            );
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
            global_direction = (receive_station->position - launch_station->position).normalize();
            
            // Convert to local coordinates for each station
            launch_local_direction = launch_station->globalToLocal(global_direction);
            receive_local_direction = receive_station->globalToLocal(-global_direction);
            
            // Check safety: direction must point away from planet surface (positive k component)
            safe_launch = launch_local_direction.z() > 0;  // k component > 0
            safe_receive = receive_local_direction.z() > 0; // k component > 0 (incoming from outside)
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
                cout << "\nADVERTENCIA: La materia llega desde el interior del planeta receptor. OPERACION PELIGROSA." << endl;
            }
            if (safe_launch && safe_receive) {
                cout << "\nConexion segura establecida." << endl;
            }
        }
};

int main() {
    try {
        cout << "=== SISTEMA DE CONTROL DE CATAPULTA CUANTICA FTL ===" << endl;
        cout << "FTL Dynamics - Tecnologia hasta 20c" << endl;
        
        // Input for first planet
        cout << "\n--- PLANETA DE LANZAMIENTO ---" << endl;
        cout << "Ingrese centro del planeta (x y z): ";
        double cx1, cy1, cz1;
        cin >> cx1 >> cy1 >> cz1;
        Point center1(cx1, cy1, cz1);
        
        cout << "Ingrese eje del planeta (dx dy dz): ";
        double ax1, ay1, az1;
        cin >> ax1 >> ay1 >> az1;
        Direction axis1(ax1, ay1, az1);
        
        cout << "Ingrese ciudad de referencia (x y z): ";
        double rx1, ry1, rz1;
        cin >> rx1 >> ry1 >> rz1;
        Point city_ref1(rx1, ry1, rz1);
        
        Planet planet1(center1, axis1, city_ref1);
        
        cout << "Ingrese inclinación y azimut de la estación (theta phi en radianes): ";
        double inc1, azimut1;
        cin >> inc1 >> azimut1;
        Station station1(planet1, inc1, azimut1);
        
        // Input for second planet
        cout << "\n--- PLANETA RECEPTOR ---" << endl;
        cout << "Ingrese centro del planeta (x y z): ";
        double cx2, cy2, cz2;
        cin >> cx2 >> cy2 >> cz2;
        Point center2(cx2, cy2, cz2);
        
        cout << "Ingrese eje del planeta (dx dy dz): ";
        double ax2, ay2, az2;
        cin >> ax2 >> ay2 >> az2;
        Direction axis2(ax2, ay2, az2);
        
        cout << "Ingrese ciudad de referencia (x y z): ";
        double rx2, ry2, rz2;
        cin >> rx2 >> ry2 >> rz2;
        Point city_ref2(rx2, ry2, rz2);
        
        Planet planet2(center2, axis2, city_ref2);
        
        cout << "Ingrese inclinación y azimut de la estación (theta phi en radianes): ";
        double inc2, azimut2;
        cin >> inc2 >> azimut2;
        Station station2(planet2, inc2, azimut2);
        
        // Create and display connection
        cout << "\n--- CALCULANDO CONEXIÓN ---" << endl;
        Connection connection(&station1, &station2);
        connection.printConnection();
        
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}