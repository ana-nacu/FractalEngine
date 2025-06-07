#include "../licenta-cgal/src/MeshOptimizer.hpp"
#include <dirent.h>
#include <iostream>
#include <string>

int main() {
    const char* dirPath = "lsysGrammar/Catalog_opt";
    DIR* dir = opendir(dirPath);
    if (!dir) {
        std::cerr << "❌ Cannot open directory: " << dirPath << std::endl;
        return 1;
    }
    struct dirent* de;
    while ((de = readdir(dir)) != nullptr) {
        std::string fileName(de->d_name);
        if (fileName.size() < 4 || fileName.substr(fileName.size() - 4) != ".obj")
            continue;
        std::string path = std::string(dirPath) + "/" + fileName;
        std::cout << "Loading mesh: " << path << std::endl;
        try {
            MeshOptimizer opt(path);
        } catch (const std::exception& e) {
            std::cerr << "❌ Fail loading: " << path << " (" << e.what() << ")\n";
            closedir(dir);
            return 1;
        }
    }
    closedir(dir);
    std::cout << "✅ All optimized meshes loaded OK\n";
    return 0;
}