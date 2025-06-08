#include <catch2/catch.hpp>
#include <sys/stat.h>
#include "MeshOptimizer.hpp"  // from licenta-cgal/src

// very simple file-exists check
static bool file_exists(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0;
}

TEST_CASE("MeshOptimizer can read a simple OBJ", "[mesh]") {
    // path relative to build dir
    const std::string in = "../lsysGrammar/Catalog/Tree__standard__iter_0.obj";
    REQUIRE( file_exists(in) );                      // file must exist
    REQUIRE_NOTHROW( MeshOptimizer(in) );            // constructor should not throw
}