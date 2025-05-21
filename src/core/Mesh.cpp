#include "Mesh.h"
#include <fstream>
#include <sstream>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
Mesh::Mesh() : VAO(0), VBO(0), NBO(0), EBO(0) {}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &NBO);
    glDeleteBuffers(1, &EBO);
}

bool Mesh::loadFromOBJ(const std::string& path) {
    vertices.clear();
    normals.clear();
    indices.clear();

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "❌ Nu s-a putut deschide fișierul OBJ: " << path << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec3> temp_normals;
    std::vector<unsigned int> temp_indices;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            glm::vec3 v;
            iss >> v.x >> v.y >> v.z;
            temp_vertices.push_back(v);
        } else if (type == "vn") {
            glm::vec3 n;
            iss >> n.x >> n.y >> n.z;
            temp_normals.push_back(n);
        } else if (type == "f") {
            std::string token;
            int vIdx, nIdx;

            // procesăm fața ca triunghi
            for (int i = 0; i < 3; ++i) {
                iss >> token;

                if (sscanf(token.c_str(), "%d//%d", &vIdx, &nIdx) == 2) {
                    vertices.push_back(temp_vertices[vIdx - 1]);

                    glm::vec3 n = temp_normals[nIdx - 1];

                    normals.push_back(n);

                    indices.push_back(static_cast<unsigned int>(indices.size()));
                } else {
                    std::cerr << "❌ Format față invalid: " << token << std::endl;
                }
            }
        }
    }

    setupMesh();
    return true;
}

bool Mesh::loadFromOBJWithNormalsDebug(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "❌ Eroare la deschiderea fișierului: " << path << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec3> temp_normals;
    vertices.clear();
    normals.clear();
    indices.clear();

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            glm::vec3 v;
            iss >> v.x >> v.y >> v.z;
            temp_vertices.push_back(v);
        } else if (type == "vn") {
            glm::vec3 n;
            iss >> n.x >> n.y >> n.z;
            temp_normals.push_back(n);
        } else if (type == "f") {
            unsigned int vIdx[3], nIdx[3];
            for (int i = 0; i < 3; ++i) {
                std::string token;
                iss >> token;
                sscanf(token.c_str(), "%u//%u", &vIdx[i], &nIdx[i]);

                glm::vec3 v = temp_vertices[vIdx[i] - 1];
                glm::vec3 n = glm::normalize(temp_normals[nIdx[i] - 1]);

                vertices.push_back(v);
                normals.push_back(n);
                indices.push_back(static_cast<unsigned int>(indices.size()));
            }
        }
    }

    setupMesh();
//    std::cout << "✅ Import completat (cu debug): " << path << std::endl;
    return true;
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Mesh::draw(Shader& shader) {
    shader.use();
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}