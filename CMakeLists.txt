cmake_minimum_required(VERSION 3.28)
project(FractalEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")  # Compatibilitate macOS

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(CGAL REQUIRED COMPONENTS Core)
find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)
include_directories(/opt/homebrew/include)
# Adaugă directorul vendor pentru GLAD
include_directories(${PROJECT_SOURCE_DIR}/vendor/glad/include)

add_executable(FractalEngine
        src/main.cpp
        src/core/Window.cpp
        src/core/Shader.cpp
        src/core/Camera.cpp  # ✅ Adăugat Camera.cpp
        src/core/LSystem.cpp
        src/core/FractalRenderer.cpp
        src/core/Mesh.cpp
        vendor/glad/src/glad.c
)

target_link_libraries(FractalEngine glfw OpenGL::GL)

# Include the CGAL optimizer project so mesh_opt is built here:
add_subdirectory(licenta-cgal)

add_custom_target(optimize_assets ALL
        COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/optimize_all.py
        DEPENDS mesh_opt
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_executable(test_load
    src/test_load.cpp
    src/core/Mesh.cpp
    src/core/Shader.cpp
    vendor/glad/src/glad.c
)
target_link_libraries(test_load
    PUBLIC
      glfw
      OpenGL::GL
)