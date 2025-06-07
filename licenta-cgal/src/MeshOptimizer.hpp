#pragma once

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/IO/OBJ.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Polygon_mesh_processing/remesh.h>
#include <CGAL/Polygon_mesh_processing/repair.h>
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Midpoint_placement.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_count_ratio_stop_predicate.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>

#include <string>
#include <cmath>
#include <stdexcept>
#include <fstream>
#include <iomanip>
#include <iostream>

// alias‑uri la nivel global (nu pot fi în interiorul clasei)
namespace PMP = CGAL::Polygon_mesh_processing;
namespace SMS = CGAL::Surface_mesh_simplification;

class MeshOptimizer
{
    using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
    using Point  = Kernel::Point_3;
    using Mesh   = CGAL::Surface_mesh<Point>;

public:
    // --- Constructor: încearcă întâi citirea directă într‑un Surface_mesh;
    //     dacă eșuează (ex. fețe quad), citește "polygon soup" și triangulează
    explicit MeshOptimizer(const std::string& file_in)
    {
        // Tentativă 1: lăsăm CGAL să citească direct mesh‑ul (presupune f‑uri triunghiuri)
        if (CGAL::IO::read_OBJ(file_in, mesh_))
            return;

        // Tentativă 2: citim ca "polygon soup"
        std::vector<Point>                        points;
        std::vector<std::vector<std::size_t>>     polygons;

        if (!CGAL::IO::read_OBJ(file_in, points, polygons))
            throw std::runtime_error("Nu pot citi .obj-ul (nici ca polygon soup): " + file_in);

        // Convertim poligoanele (pot fi quad sau N‑gon) în triunghiuri
        std::vector<std::array<std::size_t, 3>> triangles;
        for (const auto& poly : polygons) {
            if (poly.size() < 3) continue;
            for (std::size_t i = 2; i < poly.size(); ++i)
                triangles.push_back({ poly[0], poly[i - 1], poly[i] });
        }

        // Orientăm & transformăm în Surface_mesh
        PMP::orient_polygon_soup(points, triangles);
        CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(points, triangles, mesh_);

        if (mesh_.number_of_faces() == 0)
            throw std::runtime_error("Polygon soup nu a putut fi convertit: " + file_in);
    }

    // --- Repară topologia şi triangulează ---
    void repair()
    {
        PMP::remove_isolated_vertices(mesh_);
        PMP::triangulate_faces(mesh_);
        // Note: orient_polygon_soup() is for raw soup data, not Surface_mesh.
        // Surface_mesh orientation is assumed correct after triangulation.
    }

    // --- Remeshing izotropic pentru triunghiuri uniforme ---
    void isotropic_remesh(double ratio = 0.02)
    {
        try {
            double target = ratio * std::sqrt(PMP::area(mesh_));
            PMP::isotropic_remeshing(
                mesh_.faces(),
                target,
                mesh_,
                CGAL::parameters::number_of_iterations(3));
        }
        catch (const CGAL::Assertion_exception& e) {
            std::cerr << "⚠️ Remeshing failed: " << e.what() << std::endl;
        }
    }

    // --- Simplificare edge‑collapse ---
    void simplify(double edge_ratio = 0.2)
    {
        SMS::Edge_count_ratio_stop_predicate<Mesh> stop(edge_ratio);

        SMS::edge_collapse(
            mesh_,
            stop,
            CGAL::parameters::
                get_cost(SMS::Edge_length_cost<Mesh>{})
                .get_placement(SMS::Midpoint_placement<Mesh>{}));
    }

    // --- Scriere OBJ cu normale ---
    // --- Scriere OBJ + vn ---
    void save(const std::string& file_out);

private:
    Mesh mesh_;
};

// ==== IMPLEMENTARE save() CU vn =================================================
inline void MeshOptimizer::save(const std::string& file_out)
{
    // 0) Alias-uri locale
    using Point  = MeshOptimizer::Point;
    using Vector = MeshOptimizer::Kernel::Vector_3;

    // 1) Extragem punctele într-un vector în ordinea index → id
    std::vector<Point> points;
    points.reserve(mesh_.number_of_vertices());
    std::vector<std::size_t> vindex(mesh_.num_vertices());
    std::size_t vid = 0;
    for (auto v : mesh_.vertices()) {
        points.push_back(mesh_.point(v));
        vindex[v] = vid++;
    }

    // 2) Adunăm fețele (triunghiuri) ca triplete de indici
    std::vector<std::array<std::size_t,3>> faces;
    faces.reserve(mesh_.number_of_faces());
    for (auto f : mesh_.faces()) {
        auto h = mesh_.halfedge(f);
        faces.push_back({
                vindex[mesh_.target(h)],
                vindex[mesh_.target(mesh_.next(h))],
                vindex[mesh_.target(mesh_.prev(h))]
        });
    }

    // 3) Calculăm normalele pe vârfuri
    std::vector<Vector> vnormals(points.size(), Vector(0,0,0));
    for (const auto& tri : faces) {
        const Point& p0 = points[tri[0]];
        const Point& p1 = points[tri[1]];
        const Point& p2 = points[tri[2]];
        Vector n = CGAL::cross_product(p1 - p0, p2 - p0);
        vnormals[tri[0]] += n;
        vnormals[tri[1]] += n;
        vnormals[tri[2]] += n;
    }
    for (auto& n : vnormals)
        if (n != Vector(0,0,0))
            n = n / std::sqrt(n.squared_length());

    // 4) Scriem OBJ manual: v, vn, f i//i
    std::ofstream out(file_out);
    if (!out)
        throw std::runtime_error("Nu pot deschide " + file_out);

    out << std::fixed << std::setprecision(8);

    // v ...
    for (const Point& p : points)
        out << "v " << p.x() << ' ' << p.y() << ' ' << p.z() << '\n';

    // vn ...
    for (const Vector& n : vnormals)
        out << "vn " << n.x() << ' ' << n.y() << ' ' << n.z() << '\n';

    // f  (1‑based indices, with double slash syntax vertex//normal)
    for (const auto& tri : faces)
        out << "f "
            << tri[0] + 1 << "//" << tri[0] + 1 << ' '
            << tri[1] + 1 << "//" << tri[1] + 1 << ' '
            << tri[2] + 1 << "//" << tri[2] + 1 << '\n';

    if (!out)
        throw std::runtime_error("Eroare la scrierea OBJ: " + file_out);
}