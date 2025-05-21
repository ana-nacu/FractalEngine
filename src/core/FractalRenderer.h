#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

class FractalRenderer {
public:
    unsigned int VAO, VBO;
    unsigned int leafVAO, leafVBO;
    std::vector<float> vertexData;  // ✅ Poziții + normale într-un singur vector
    std::vector<float> mountainVertexData;
    std::vector<float> leafData;  // ✅ Poziții + normale într-un singur vector
    int level=1;

    FractalRenderer();
    ~FractalRenderer();

    void generate();
    void generateIFS(int iterations);
    void generateMeshFromIFS();

    void draw();
    void drawLeaves();
    void generateIFSwithmatrix(const std::vector<std::vector<float>>& ifsMatrix, int iterations) ;
    void generateLSystemTree(int depth);
    void generateFernLSystem(int depth);
    void generateLSystemTree3D(const std::string& ruleString, const std::string& ruleType);
    void generateFractalMountainSphere(int subdivisions, float radius);
    void generateFractalCubeMountain(float radius, int resolution);
    void generateSimpleSphere(float radius, int latBands, int longBands);

    void drawM();


private:
    void generateBranch(glm::vec3 start, glm::vec3 direction, float length, int depth);
    std::vector<glm::vec3> branchPositions;  // ✅ Stocăm pozițiile ramurilor pentru generarea cilindrilor

    void prepareLeaves();
    void generateLeaf(glm::vec3 position);
    void generateBranchfern(glm::vec3 start, glm::vec3 direction, float length, int depth);
};