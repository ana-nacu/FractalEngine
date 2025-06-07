#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Shader.h"

class Mesh {
public:
    Mesh();
    ~Mesh();

    bool loadFromOBJ(const std::string& path);
    void draw(Shader& shader);
    bool loadFromOBJWithNormalsDebug(const std::string &path);
    void setupMesh();

private:


    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    GLuint VAO, VBO, NBO, EBO;
};