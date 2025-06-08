cmake_minimum_required(VERSION 3.28)
project(FractalEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(CGAL REQUIRED COMPONENTS Core)
find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)

include_directories(${GMP_INCLUDE_DIRS} ${MPFR_INCLUDE_DIRS})
include_directories(/opt/homebrew/include)
include_directories(${PROJECT_SOURCE_DIR}/vendor/glad/include)

# Executabil demo
add_executable(FractalEngine
        src/main.cpp
        src/core/Window.cpp
        src/core/Shader.cpp
        src/core/Camera.cpp
        src/core/LSystem.cpp
        src/core/FractalRenderer.cpp
        src/core/Mesh.cpp
        vendor/glad/src/glad.c
)
target_link_libraries(FractalEngine PRIVATE glfw OpenGL::GL)

# Subproject CGAL optimizer
add_subdirectory(licenta-cgal)

# Library pentru Mesh + Shader (folosit atât de demo, cât și de teste)
add_library(mesh_lib
        src/core/Mesh.cpp
        src/core/Shader.cpp
        vendor/glad/src/glad.c
)
target_include_directories(mesh_lib PUBLIC
        ${PROJECT_SOURCE_DIR}/src/core
        ${PROJECT_SOURCE_DIR}/vendor/glad/include
)
target_link_libraries(mesh_lib PUBLIC glfw OpenGL::GL)

#––– Unit testing with Catch2 –––
include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v2.13.10
)
FetchContent_MakeAvailable(Catch2)

enable_testing()

add_executable(unit_tests
        tests/test_main.cpp
        tests/test_mesh.cpp
        tests/test_optimizer.cpp
)
target_include_directories(unit_tests PRIVATE
        ${PROJECT_SOURCE_DIR}/licenta-cgal/src
        ${PROJECT_SOURCE_DIR}/src/core
)
target_link_libraries(unit_tests PRIVATE
        Catch2::Catch2
        mesh_lib
        CGAL::CGAL
        ${GMP_LIBRARIES}
        ${MPFR_LIBRARIES}
)
# — only copy test‐meshes if the folder actually exists —
set(TEST_DATA_DIR ${CMAKE_SOURCE_DIR}/tests/data)
if(EXISTS ${TEST_DATA_DIR})
    file(COPY
            ${TEST_DATA_DIR}
            DESTINATION ${CMAKE_BINARY_DIR}/tests/data
    )
else()
    message(STATUS "No test‐meshes in ${TEST_DATA_DIR}, skipping copy.")
endif()

# — now register the test and force it to run out of the build‐dir —
add_test(NAME mesh_and_optimizer_tests
        COMMAND unit_tests
)
# --- copy test data for Catch2 unit tests ---
# (după add_test(NAME mesh_and_optimizer_tests COMMAND unit_tests))
file(COPY
        ${PROJECT_SOURCE_DIR}/tests/data
        DESTINATION
        ${CMAKE_BINARY_DIR}/tests
)
set_tests_properties(mesh_and_optimizer_tests PROPERTIES
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
# Test loader
add_executable(test_load
        src/test_load.cpp
        src/core/Mesh.cpp
        src/core/Shader.cpp
        vendor/glad/src/glad.c
)
target_link_libraries(test_load PUBLIC glfw OpenGL::GL)

