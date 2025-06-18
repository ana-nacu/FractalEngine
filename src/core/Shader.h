#pragma once

#include <string>
#include <glm/glm.hpp>
class Shader {
public:
    unsigned int ID;
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    void use();
    ~Shader();
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setInt(const std::string& name, int value) const;
};