#include <catch2/catch.hpp>
#include "MeshOptimizer.hpp"

// Helper to check file existence
#include <sys/stat.h>
static bool file_exists(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0;
}

TEST_CASE("Optimizer produces an output OBJ", "[optimizer]") {
    // Pregătim un mesh simplu de test
    const std::string in  = "tests/data/simple_triangle.obj";
    const std::string out = "tests/out/optimized_triangle.obj";

    // Creăm directorul de output (ignorăm eroarea dacă există deja)
    mkdir("tests/out", 0755);

    // Rulăm optimizarea
    MeshOptimizer opt(in);
    opt.repair();
    opt.simplify(0.5);
    opt.save(out);

    REQUIRE(file_exists(out));
}