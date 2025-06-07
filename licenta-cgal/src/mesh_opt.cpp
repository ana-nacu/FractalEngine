#include "MeshOptimizer.hpp"

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cerr << "Usage: mesh_opt <in.obj> <out.obj>\n";
        return 1;
    }

    MeshOptimizer opt(argv[1]);
    opt.repair();
    opt.isotropic_remesh();      // poți ajusta parametrii
    opt.simplify(0.15);          // păstrează 15 % din muchii
    opt.save(argv[2]);

    std::cout << "Mesh optimizat cu succes!\n";
    return 0;
}
