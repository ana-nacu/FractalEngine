cmake_minimum_required(VERSION 3.20)
project(mesh_opt LANGUAGES CXX)

# -------------------------------------------------
# 1) Setează standardul și warning-urile de bază
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)

# -------------------------------------------------
# 2) Găsește CGAL ca bibliotecă de HEADERE
#    (Homebrew instalează numai header-ele)
find_path(CGAL_INCLUDE_DIR NAMES CGAL/Simple_cartesian.h
        PATHS /opt/homebrew/opt/cgal/include)

# 3) Căi spre Boost, GMP, MPFR (Homebrew)
find_path(BOOST_INCLUDE_DIR NAMES boost/config.hpp
        PATHS /opt/homebrew/opt/boost/include)
find_library(GMP_LIBRARY   NAMES gmp   PATHS /opt/homebrew/opt/gmp/lib)
find_library(MPFR_LIBRARY  NAMES mpfr  PATHS /opt/homebrew/opt/mpfr/lib)

# -------------------------------------------------
# 4) Definește targetul executabil
add_executable(mesh_opt
        src/mesh_opt.cpp
        src/MeshOptimizer.hpp
)

# 5) Opțiune cheie: CGAL header-only
target_compile_definitions(mesh_opt PRIVATE CGAL_HEADER_ONLY)

# 6) Include-uri și librării
target_include_directories(mesh_opt PRIVATE
        ${CGAL_INCLUDE_DIR}
        ${BOOST_INCLUDE_DIR}
)

target_link_libraries(mesh_opt PRIVATE
        ${GMP_LIBRARY}
        ${MPFR_LIBRARY}
)

# (opțional) PCH = compilează mai repede template-urile grele
target_precompile_headers(mesh_opt PRIVATE
        <CGAL/Simple_cartesian.h>
        <CGAL/Surface_mesh.h>
)