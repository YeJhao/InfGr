cmake_minimum_required(VERSION 3.10)
project(InformaticaGrafica)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar librerías necesarias
find_package(OpenEXR REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(PNG REQUIRED)

# Directorio de ejecutables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/executables)

# Librería de geometría
add_library(geometry_lib STATIC
    geometry/geometry.cpp
    geometry/color.cpp
    geometry/plane.cpp
    geometry/sphere.cpp
    geometry/triangle.cpp
)

target_include_directories(geometry_lib PUBLIC 
    ${CMAKE_SOURCE_DIR}
    ${EIGEN3_INCLUDE_DIR}
)

# Librería de imaging
add_library(imaging_lib STATIC
    imaging/imaging_lib.cpp
)

target_include_directories(imaging_lib PUBLIC 
    ${CMAKE_SOURCE_DIR}
    ${PNG_INCLUDE_DIRS}
)
target_link_libraries(imaging_lib 
    OpenEXR::OpenEXR
    PNG::PNG
)

# Librería de lights
add_library(light_lib STATIC
    light/point_light.cpp
)

target_include_directories(light_lib PUBLIC 
    ${CMAKE_SOURCE_DIR}
    ${EIGEN3_INCLUDE_DIR}
)

# Librería de ray tracing (ahora en carpeta ray/)
add_library(ray_lib STATIC
    ray/ray_lib.cpp
)

target_include_directories(ray_lib PUBLIC 
    ${CMAKE_SOURCE_DIR}
    ${EIGEN3_INCLUDE_DIR}
)
target_link_libraries(ray_lib geometry_lib)

# Ejecutable principal: generate_image
add_executable(generate_image
    generate_image.cpp
)

target_include_directories(generate_image PRIVATE 
    ${CMAKE_SOURCE_DIR}
    ${EIGEN3_INCLUDE_DIR}
)
target_link_libraries(generate_image
    geometry_lib
    imaging_lib
    light_lib
    ray_lib
    OpenEXR::OpenEXR
)