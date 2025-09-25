cmake_minimum_required(VERSION 3.16)
project(InfGr)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/executables)

# Check and install required packages
function(check_and_install_package package_name apt_package)
    execute_process(
        COMMAND dpkg -l ${apt_package}
        RESULT_VARIABLE package_result
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(NOT package_result EQUAL 0)
        message(STATUS "Installing ${package_name}...")
        execute_process(
            COMMAND sudo apt install -y ${apt_package}
            RESULT_VARIABLE install_result
        )
        if(install_result EQUAL 0)
            message(STATUS "${package_name} installed successfully")
        else()
            message(FATAL_ERROR "Failed to install ${package_name}")
        endif()
    else()
        message(STATUS "${package_name} is already installed")
    endif()
endfunction()

# Check and install required packages
check_and_install_package("CMake" "cmake")
check_and_install_package("libpng-dev" "libpng-dev")
check_and_install_package("libopenexr-dev" "libopenexr-dev")
check_and_install_package("libeigen3-dev" "libeigen3-dev")

# Find required packages
find_package(PNG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(OPENEXR REQUIRED OpenEXR)
find_package(Eigen3 REQUIRED)

# Create executable for geometry.cpp
add_executable(geometry ${CMAKE_CURRENT_SOURCE_DIR}/geometry/geometry.cpp)
target_link_libraries(geometry 
    PNG::PNG
    ${OPENEXR_LIBRARIES}
    Eigen3::Eigen
)
target_include_directories(geometry PRIVATE
    ${OPENEXR_INCLUDE_DIRS}
)

# Create executable for imaging.cpp
add_executable(imaging ${CMAKE_CURRENT_SOURCE_DIR}/imaging/imaging.cpp)
target_link_libraries(imaging 
    PNG::PNG
    ${OPENEXR_LIBRARIES}
)
target_include_directories(imaging PRIVATE
    ${OPENEXR_INCLUDE_DIRS}
)

message(STATUS "Executables will be placed in: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "Building geometry and imaging executables")