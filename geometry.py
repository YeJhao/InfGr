import numpy as np

class Point:
    # Origen (0,0)
    def __init__(self, x, y, z):
        self.coords = np.array([x, y, z], dtype=float)

    def __sub__(self, other):
        return Direction(*(self.coords - other.coords))

class Direction:
    def __init__(self, d_x, d_y, d_z):
        self.d = np.array([d_x, d_y, d_z], dtype=float)

    # Cálculo de módulo de un vector
    def mod(self):
        return np.linalg.norm(self)

    def normalize(self):
        return Direction(*(self.d / self.mod()))

class Planet:
    def __init__(self, center: Point, axis: Direction, city_ref: Point):
        # Radio según el eje
        radius_from_axis = axis.mod() / 2
        # Radio según la ciudad de referencia
        radius_from_city = (city_ref - center).mod()

        if abs(radius_from_axis - radius_from_city) > 1e-6:
            raise ValueError("Radio inconsistente")
        
        # "center" es un vector de tipo "Point"
        self.center = center
        # "axis" es un vector de tipo "Direction"
        self.axis = axis.normalize()
        # "city_ref" es un vector de tipo "Point"
        self.city_ref = city_ref
        # "radio" es un escalar
        self.radius = radius_from_axis

        # Calculamos el azimut0, que gira en torno al ecuador del planeta y es perpendicular al eje y 
        #   apunta al meridiano
        # Vector de centro a ciudad
        center_to_city = Direction(self.city_ref - self.center).normalize()

        # Perpendicular al eje y al vector que une centro con ciudad de referencia
        ecuadorial = np.cross(self.axis.d, center_to_city.d).normalize()
        
        # Cálculo del azimut0
        self.azimut0 = (np.cross(ecuadorial.d, self.axis.d)).normalize()


class Station:
    def __init__(self, planet: Planet, inclination, azimuth):
        self.inclination = inclination  # Radianes (0, π)
        self.azimuth = azimuth  # Radianes (−π, π]
        self.planet = planet

        # Cálculo del punto de la estación
        x = planet.radius * np.sin(inclination) * np.sin(azimuth)
        y = planet.radius * np.sin(inclination) * np.cos(azimuth)
        z = planet.radius * np.cos(inclination)

        self.position = Point(planet.center.coords[0] + x, planet.center.coords[1] + y, planet.center.coords[2] + z)

        # Vector normal a la superficie del planeta
        self.k = (self.position - planet.center).normalize()  # Vector unitario desde el centro del planeta a la estación
        self.i = np.cross(planet.axis.d, self.k.d).normalize() # Vector unitario, tangente a la superficie y perpendicular al eje
        self.j = np.cros(self.i.d, self.k.d).normalize() # Vector unitario, tangente a la superficie y perpendicular a i y k
        