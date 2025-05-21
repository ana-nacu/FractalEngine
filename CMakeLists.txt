cmake_minimum_required(VERSION 3.28)
project(FractalEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")  # Compatibilitate macOS

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
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
