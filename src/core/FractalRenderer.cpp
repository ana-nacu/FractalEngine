#include "FractalRenderer.h"
#include "LSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stack>
#include <regex>  // Adăugat pentru regex parsing
#include <glm/gtc/noise.hpp> // Noise din GLM (Simplex/Perlin)
#include <map>

void checkOpenGLError(const std::string& message) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "❌ OpenGL ERROR " << err << " after: " << message << std::endl;
    }
}
FractalRenderer::FractalRenderer() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

FractalRenderer::~FractalRenderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void FractalRenderer::generate() {
    vertexData.clear();

    glm::vec3 start(0.0f, -0.5f, 0.0f);
    glm::vec3 direction(0.0f, 1.0f, 0.0f);
    float length = 0.7f;

    generateBranch(start, direction, length, level);

//    std::cout << "=== DEBUG: Verificăm datele generate ===\n";
//    for (size_t i = 0; i < vertexData.size(); i += 6) {
//        std::cout << "Vertex: (" << vertexData[i] << ", " << vertexData[i+1] << ", " << vertexData[i+2] << ") ";
//        std::cout << "| Normal: (" << vertexData[i+3] << ", " << vertexData[i+4] << ", " << vertexData[i+5] << ")\n";
//    }
//    std::cout << "=== SFÂRȘIT DEBUG ===\n";
    prepareLeaves();
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void FractalRenderer::generateLSystemTree3D(const std::string& ruleString, const std::string& ruleType) {
    vertexData.clear();

    std::regex paramRegex(R"(([A-Z])\((-?[0-9]*\.?[0-9]+)\))");
    std::smatch match;

    glm::vec3 position(0.0f, 0.0f, 0.0f);
    glm::vec3 direction(0.0f, 1.0f, 0.0f);
    std::stack<std::pair<glm::vec3, glm::vec3>> stateStack;
    std::vector<std::pair<glm::vec3, glm::vec3>> branches;

    float angle = glm::radians(30.0f);
    bool isParametric = (ruleType == "parametric");

    std::string result = ruleString;
    std::string::const_iterator it = result.begin();

    while (it != result.end()) {
        std::string::const_iterator searchStart = it;
        // Dacă nu e literă parametrică, tratăm normal
        char c = *it;

        if ((c == 'F' || c == 'X')&& !isParametric) {
            float defaultLen = 0.05f;
            glm::vec3 newPos = position + direction * defaultLen;
            branches.push_back({position, newPos});
            position = newPos;
            ++it;

        } else if (c == '+') {
            direction = glm::mat3(glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1))) * direction;
            ++it;

        } else if (c == '-') {
            direction = glm::mat3(glm::rotate(glm::mat4(1.0f), -angle, glm::vec3(0, 0, 1))) * direction;
            ++it;

        } else if (c == '^') {
            direction = glm::mat3(glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1, 0, 0))) * direction;
            ++it;

        } else if (c == '&') {
            direction = glm::mat3(glm::rotate(glm::mat4(1.0f), -angle, glm::vec3(1, 0, 0))) * direction;
            ++it;

        } else if (c == '[') {
            stateStack.push({position, direction});
            ++it;

        } else if (c == ']') {
            if (!stateStack.empty()) {
                position = stateStack.top().first;
                direction = stateStack.top().second;
                stateStack.pop();
            }
            ++it;

        }

        // Dacă e parametric și se potrivește de la poziția curentă
        if (isParametric && std::regex_search(searchStart, result.cend(), match, paramRegex) && match.prefix().first == it) {
            std::string symbol = match[1];
            float param = std::stof(match[2]);

            if ((symbol == "F") || (symbol == "A")) {
                glm::vec3 newPos = position + direction * param;
                branches.push_back({position, newPos});
                position = newPos;
            }

            it = match[0].second;
            continue;
        }


    }

    for (const auto& branch : branches) {
        generateBranch(branch.first, glm::normalize(branch.second - branch.first), glm::length(branch.second - branch.first), 1);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    if (vertexData.empty()) {
        std::cerr << "⚠️ Vertex data is empty! Nothing will be drawn!" << std::endl;
    }
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
void FractalRenderer::generateFernLSystem(int depth) {
    vertexData.clear();

    //  Definim L-System pentru ferigă
    LSystem lsystem("X", depth);
    lsystem.addRule('X', "F+[[X]-X]-F[-FX]+X");
    lsystem.addRule('F', "FF");

    std::string result = lsystem.generate();

    //  Interpretăm string-ul generat pentru a construi geometria
    glm::vec3 position(5.0f, 0.0f, 0.0f);  // Poziția de start
    glm::vec3 direction(0.0f, 1.0f, 0.0f); // Direcția inițială (în sus)
    std::stack<std::pair<glm::vec3, glm::vec3>> stateStack;

    float length = 0.05f;  // Lungimea inițială a fiecărei ramuri

    float scaleFactor = 0.7f;  // Factor de scalare pe măsură ce urcăm în structură
    float angle = glm::radians(20.0f);  // Unghi de ramificare

    std::vector<std::pair<glm::vec3, glm::vec3>> branches;  //  Stocăm segmentele ca să le desenăm corect

    for (char c : result) {
        if (c == 'F') {
            glm::vec3 newPos = position + direction * length;
            branches.push_back({position, newPos}); //  Adăugăm segmentul
            position = newPos;
        }
        else if (c == '+') {
            direction = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1)) * glm::vec4(direction, 1.0f);
        }
        else if (c == '-') {
            direction = glm::rotate(glm::mat4(1.0f), -angle, glm::vec3(0, 0, 1)) * glm::vec4(direction, 1.0f);
        }
        else if (c == '[') {
            stateStack.push({position, direction});
            length = glm::max(length * scaleFactor, 0.02f); // Evităm ca lungimea să devină prea mică
        }
        else if (c == ']') {
            if (!stateStack.empty()) {
                position = stateStack.top().first;
                direction = stateStack.top().second;
                stateStack.pop();
            }
        }
    }
//std::cout<<branches.size()<<std::endl;
    //  Acum desenăm toate ramurile salvate în `branches`
    for (const auto& branch : branches) {
        generateBranchfern(branch.first, glm::normalize(branch.second - branch.first), length, 1);
    }

    //  Transferă datele la GPU
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    std::cout << "VertexData size: " << vertexData.size() << std::endl;
    if (vertexData.empty()) {
        std::cerr << "⚠️ Vertex data is empty! Nothing will be drawn!" << std::endl;
    }
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
//void FractalRenderer::generateFernLSystem(int depth) {
//    vertexData.clear();
//
//    //  Definim L-System pentru ferigă
//    LSystem lsystem("X", depth);
//    lsystem.addRule('X', "F+[[X]-X]-F[-FX]+X");
//    lsystem.addRule('F', "FF");
//
//    std::string result = lsystem.generate();
//
//    //  Interpretăm string-ul generat pentru a construi geometria
//    glm::vec3 position(0.0f, 0.0f, 0.0f);
//    glm::vec3 direction(0.0f, 1.0f, 0.0f);
//    std::stack<std::pair<glm::vec3, glm::vec3>> stateStack;
//
//    float length = 0.05f;  // Lungimea inițială a fiecărei ramuri
//    float scaleFactor = 0.7f;  // Factor de scalare pe măsură ce urcăm în structură
//    float angle = glm::radians(25.0f);  // Unghi de ramificare
//
//    for (char c : result) {
//        if (c == 'F') {
//            glm::vec3 newPos = position + direction * length;
//            generateBranch(position, direction, length, 1); // Creează cilindri pentru ramuri
//            position = newPos;
//        } else if (c == '+') {
//            direction = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1)) * glm::vec4(direction, 1.0f);
//        } else if (c == '-') {
//            direction = glm::rotate(glm::mat4(1.0f), -angle, glm::vec3(0, 0, 1)) * glm::vec4(direction, 1.0f);
//        } else if (c == '[') {
//            stateStack.push({position, direction});
//            length *= scaleFactor;  // Reducem dimensiunea pentru ramurile superioare
//        } else if (c == ']') {
//            if (!stateStack.empty()) {
//                position = stateStack.top().first;
//                direction = stateStack.top().second;
//                stateStack.pop();
//            }
//        }
//    }
//
//    //  Transferă datele la GPU
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
//    glEnableVertexAttribArray(1);
//}

void FractalRenderer::generateLSystemTree(int depth) {
    vertexData.clear();

    // Creează un L-System pentru un copac
//    LSystem lsystem("X", depth);
//    lsystem.addRule('X', "F[+X][-X]FX");
//    lsystem.addRule('F', "FF");
    LSystem lsystem("X", depth);
    lsystem.addRule('X', "F[+XZ][-XZ][^YZ][&YZ]FX"); // Ramuri în toate planurile
    lsystem.addRule('F', "F[&FX]");  // Scalare mai agresivă, reduce creșterea trunchiului

    std::string result = lsystem.generate();

    //  Interpretăm string-ul generat pentru a construi geometria
    glm::vec3 position(0.0f, 0.0f, 1.0f);  // Poziția de start
    glm::vec3 direction(0.0f, 1.0f, 0.0f); // Direcția inițială (în sus)
    std::stack<std::pair<glm::vec3, glm::vec3>> stateStack;

    float length = 0.05f;  // Lungimea inițială a fiecărei ramuri

    float scaleFactor = 0.7f;  // Factor de scalare pe măsură ce urcăm în structură
    float angle = glm::radians(20.0f);  // Unghi de ramificare

    std::vector<std::pair<glm::vec3, glm::vec3>> branches;  //  Stocăm segmentele ca să le desenăm corect

    for (char c : result) {

        if (c == 'F') {
            glm::vec3 newPos = position + direction * length;
            branches.push_back({position, newPos}); //  Adăugăm segmentul
            position = newPos;
        }
        else if (c == 'X') {
            direction = glm::rotate(glm::mat4(1.0f), glm::radians(25.0f), glm::vec3(1, 0, 0)) * glm::vec4(direction, 1.0f);
        }
        else if (c == 'Z') {
            direction = glm::rotate(glm::mat4(1.0f), glm::radians(-25.0f), glm::vec3(0, 0, 1)) * glm::vec4(direction, 1.0f);
        }
        else if (c == '^') {
            direction = glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0, 1, 0)) * glm::vec4(direction, 1.0f);
        }
        else if (c == '&') {
            direction = glm::rotate(glm::mat4(1.0f), glm::radians(-15.0f), glm::vec3(0, 1, 0)) * glm::vec4(direction, 1.0f);
        }else if (c == '+') {
            direction = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1)) * glm::vec4(direction, 1.0f);
        }
        else if (c == '-') {
            direction = glm::rotate(glm::mat4(1.0f), -angle, glm::vec3(0, 0, 1)) * glm::vec4(direction, 1.0f);
        }
        else if (c == '[') {
            stateStack.push({position, direction});
            length = glm::max(length * scaleFactor, 0.02f); // Evităm ca lungimea să devină prea mică
        }
        else if (c == ']') {
            if (!stateStack.empty()) {
                position = stateStack.top().first;
                direction = stateStack.top().second;
                stateStack.pop();
            }
        }
    }
    //std::cout<<branches.size()<<std::endl;
    //  Acum desenăm toate ramurile salvate în `branches`
    for (const auto& branch : branches) {
        generateBranchfern(branch.first, glm::normalize(branch.second - branch.first), length, 1);
    }

    //  Transferă datele la GPU
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //    std::cout << "VertexData size: " << vertexData.size() << std::endl;
    if (vertexData.empty()) {
        std::cerr << "⚠️ Vertex data is empty! Nothing will be drawn!" << std::endl;
    }
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
void FractalRenderer::generateBranch(glm::vec3 start, glm::vec3 direction, float length, int depth) {
    if (depth == 0) {
        generateLeaf(start);  //  Adăugăm frunză la capătul fiecărei ramuri finale
        return;
    }

    glm::vec3 end = start + direction * length;

    float radius = length * 0.05f;
    int numSides = 6;

    for (int i = 0; i < numSides; i++) {
        float theta1 = (float)i / numSides * 2.0f * glm::pi<float>();
        float theta2 = (float)(i + 1) / numSides * 2.0f * glm::pi<float>();

        glm::vec3 p1 = start + radius * glm::vec3(cos(theta1), sin(theta1), 0.0f);
        glm::vec3 p2 = start + radius * glm::vec3(cos(theta2), sin(theta2), 0.0f);
        glm::vec3 p3 = end + radius * glm::vec3(cos(theta1), sin(theta1), 0.0f);
        glm::vec3 p4 = end + radius * glm::vec3(cos(theta2), sin(theta2), 0.0f);

        glm::vec3 normal1 = glm::normalize(glm::cross(p3 - p1, p2 - p1));
        glm::vec3 normal2 = glm::normalize(glm::cross(p4 - p3, p2 - p3));

        if (glm::length(normal1) < 0.1f) normal1 = glm::vec3(0.0f, 1.0f, 0.0f);
        if (glm::length(normal2) < 0.1f) normal2 = glm::vec3(0.0f, 1.0f, 0.0f);

        auto addVertex = [&](glm::vec3 pos, glm::vec3 norm) {
            vertexData.push_back(pos.x);
            vertexData.push_back(pos.y);
            vertexData.push_back(pos.z);
            vertexData.push_back(norm.x);
            vertexData.push_back(norm.y);
            vertexData.push_back(norm.z);
        };

        addVertex(p1, normal1);
        addVertex(p2, normal1);
        addVertex(p3, normal1);

        addVertex(p3, normal2);
        addVertex(p2, normal2);
        addVertex(p4, normal2);
    }

    float angleXY = glm::radians(30.0f);
    float angleXZ = glm::radians(20.0f);

    glm::mat3 rotXY_Left = glm::mat3(glm::rotate(glm::mat4(1.0f), angleXY, glm::vec3(0.0f, 0.0f, 1.0f)));
    glm::mat3 rotXY_Right = glm::mat3(glm::rotate(glm::mat4(1.0f), -angleXY, glm::vec3(0.0f, 0.0f, 1.0f)));
    glm::mat3 rotXZ_Forward = glm::mat3(glm::rotate(glm::mat4(1.0f), angleXZ, glm::vec3(1.0f, 0.0f, 0.0f)));
    glm::mat3 rotXZ_Backward = glm::mat3(glm::rotate(glm::mat4(1.0f), -angleXZ, glm::vec3(1.0f, 0.0f, 0.0f)));

    generateBranch(end, rotXY_Left * direction, length * 0.7f, depth - 1);
    generateBranch(end, rotXY_Right * direction, length * 0.7f, depth - 1);
    generateBranch(end, rotXZ_Forward * direction, length * 0.7f, depth - 1);
    generateBranch(end, rotXZ_Backward * direction, length * 0.7f, depth - 1);
}

void FractalRenderer::generateBranchfern(glm::vec3 start, glm::vec3 direction, float length, int depth) {

    if (depth == 0)
        return; // Nu generăm mai departe dacă am atins adâncimea maximă

    glm::vec3 end = start + direction * length;
//    std::cout << "Generating branch from " << start.x << ", " << start.y << ", " << start.z
//              << " to " << end.x << ", " << end.y << ", " << end.z << std::endl;
    float radius = length * 0.03f;  //  Ferigile sunt mai subțiri
    radius = glm::max(radius, 0.001f); // Evităm valori prea mici
    int numSides = 4;  //  Facem un cvadrilateral, nu un hexagon (mai natural)

    for (int i = 0; i < numSides; i++) {
        float theta1 = (float)i / numSides * glm::pi<float>() * 2.0f;
        float theta2 = (float)(i + 1) / numSides * glm::pi<float>() * 2.0f;

        glm::vec3 p1 = start + radius * glm::vec3(cos(theta1), sin(theta1), 0.0f);
        glm::vec3 p2 = start + radius * glm::vec3(cos(theta2), sin(theta2), 0.0f);
        glm::vec3 p3 = end + radius * glm::vec3(cos(theta1), sin(theta1), 0.0f);
        glm::vec3 p4 = end + radius * glm::vec3(cos(theta2), sin(theta2), 0.0f);

        glm::vec3 normal1 = glm::normalize(glm::cross(p3 - p1, p2 - p1));
        glm::vec3 normal2 = glm::normalize(glm::cross(p4 - p3, p2 - p3));

        auto addVertex = [&](glm::vec3 pos, glm::vec3 norm) {
            vertexData.push_back(pos.x);
            vertexData.push_back(pos.y);
            vertexData.push_back(pos.z);
            vertexData.push_back(norm.x);
            vertexData.push_back(norm.y);
            vertexData.push_back(norm.z);
        };

        addVertex(p1, normal1);
        addVertex(p2, normal1);
        addVertex(p3, normal1);

        addVertex(p3, normal2);
        addVertex(p2, normal2);
        addVertex(p4, normal2);
    }

    //  Aplicăm o **ușoară curbură** spre dreapta, specifică ferigilor
    float curveAngle = glm::radians(5.0f * depth);
    glm::mat3 curve = glm::mat3(glm::rotate(glm::mat4(1.0f), curveAngle, glm::vec3(0.0f, 0.0f, 1.0f)));

    //  Generăm ramurile într-o singură direcție (NU simetric ca la copaci)
    float angleXZ = glm::radians(15.0f);  //  Unghi mic, pentru creștere lentă
    glm::mat3 rotXZ_Forward = glm::mat3(glm::rotate(glm::mat4(1.0f), angleXZ, glm::vec3(1.0f, 0.0f, 0.0f)));

    generateBranch(end, curve * rotXZ_Forward * direction, length * 0.8f, depth - 1);
}
void FractalRenderer::generateLeaf(glm::vec3 position) {
    float size = 0.05f;  //  Dimensiunea frunzei
    glm::vec3 normal(0.0f, 1.0f, 0.0f);

    //  Definim trei puncte pentru un triunghi
    glm::vec3 p1 = position + glm::vec3(-size, 0.0f, 0.0f);
    glm::vec3 p2 = position + glm::vec3(size, 0.0f, 0.0f);
    glm::vec3 p3 = position + glm::vec3(0.0f, size, 0.0f);

    auto addVertex = [&](glm::vec3 pos, glm::vec3 norm) {
        leafData.push_back(pos.x);
        leafData.push_back(pos.y);
        leafData.push_back(pos.z);
        leafData.push_back(norm.x);
        leafData.push_back(norm.y);
        leafData.push_back(norm.z);
    };

    addVertex(p1, normal);
    addVertex(p2, normal);
    addVertex(p3, normal);
}
void FractalRenderer::prepareLeaves() {
    glGenVertexArrays(1, &leafVAO);
    glGenBuffers(1, &leafVBO);

    glBindVertexArray(leafVAO);
    glBindBuffer(GL_ARRAY_BUFFER, leafVBO);
    glBufferData(GL_ARRAY_BUFFER, leafData.size() * sizeof(float), leafData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
void FractalRenderer::drawLeaves() {
    glBindVertexArray(leafVAO);
    glDrawArrays(GL_TRIANGLES, 0, leafData.size() / 6);
}
//void FractalRenderer::generateBranch(glm::vec3 start, glm::vec3 direction, float length, int depth) {
//    if (depth == 0) return;
//
//    glm::vec3 end = start + direction * length;
//
//    float radius = length * 0.05f;
//    int numSides = 6;
//
//    for (int i = 0; i < numSides; i++) {
//        float theta1 = (float)i / numSides * 2.0f * glm::pi<float>();
//        float theta2 = (float)(i + 1) / numSides * 2.0f * glm::pi<float>();
//
//        glm::vec3 p1 = start + radius * glm::vec3(cos(theta1), sin(theta1), 0.0f);
//        glm::vec3 p2 = start + radius * glm::vec3(cos(theta2), sin(theta2), 0.0f);
//        glm::vec3 p3 = end + radius * glm::vec3(cos(theta1), sin(theta1), 0.0f);
//        glm::vec3 p4 = end + radius * glm::vec3(cos(theta2), sin(theta2), 0.0f);
//
//        glm::vec3 normal1 = glm::normalize(glm::cross(p3 - p1, p2 - p1));
//        glm::vec3 normal2 = glm::normalize(glm::cross(p4 - p3, p2 - p3));
//
//        if (glm::length(normal1) < 0.1f) normal1 = glm::vec3(0.0f, 1.0f, 0.0f);
//        if (glm::length(normal2) < 0.1f) normal2 = glm::vec3(0.0f, 1.0f, 0.0f);
//
//        //  Adăugăm vertecși + normale în `vertexData`
//        auto addVertex = [&](glm::vec3 pos, glm::vec3 norm) {
//            vertexData.push_back(pos.x);
//            vertexData.push_back(pos.y);
//            vertexData.push_back(pos.z);
//            vertexData.push_back(norm.x);
//            vertexData.push_back(norm.y);
//            vertexData.push_back(norm.z);
//        };
//
//        addVertex(p1, normal1);
//        addVertex(p2, normal1);
//        addVertex(p3, normal1);
//
//        addVertex(p3, normal2);
//        addVertex(p2, normal2);
//        addVertex(p4, normal2);
//    }
//
//    float angleXY = glm::radians(30.0f);
//    float angleXZ = glm::radians(20.0f);
//
//    glm::mat3 rotXY_Left = glm::mat3(glm::rotate(glm::mat4(1.0f), angleXY, glm::vec3(0.0f, 0.0f, 1.0f)));
//    glm::mat3 rotXY_Right = glm::mat3(glm::rotate(glm::mat4(1.0f), -angleXY, glm::vec3(0.0f, 0.0f, 1.0f)));
//    glm::mat3 rotXZ_Forward = glm::mat3(glm::rotate(glm::mat4(1.0f), angleXZ, glm::vec3(1.0f, 0.0f, 0.0f)));
//    glm::mat3 rotXZ_Backward = glm::mat3(glm::rotate(glm::mat4(1.0f), -angleXZ, glm::vec3(1.0f, 0.0f, 0.0f)));
//
//    generateBranch(end, rotXY_Left * direction, length * 0.7f, depth - 1);
//    generateBranch(end, rotXY_Right * direction, length * 0.7f, depth - 1);
//    generateBranch(end, rotXZ_Forward * direction, length * 0.7f, depth - 1);
//    generateBranch(end, rotXZ_Backward * direction, length * 0.7f, depth - 1);
//}
////////////////////////////////////////////////////////////////merge dar nu prea pe IFS
//void FractalRenderer::generateIFS(int iterations) {
//    vertexData.clear();
//
//    //  Set inițial de puncte (trunchiul)
//    std::vector<glm::vec3> points = { glm::vec3(0.0f, 0.0f, 0.0f) };
//
//    //  Matricile de transformare pentru copac
//    struct Transform {
//        glm::mat2 mat;
//        glm::vec2 offset;
//    };
//
//    std::vector<Transform> transforms = {
//            { glm::mat2(0.5f, 0.0f, 0.0f, 0.5f), glm::vec2(0.0f, 0.5f) },  // Trunchi
//            { glm::mat2(0.42f, -0.42f, 0.42f, 0.42f), glm::vec2(0.0f, 0.5f) },  // Ramura stângă
//            { glm::mat2(0.42f, 0.42f, -0.42f, 0.42f), glm::vec2(0.0f, 0.5f) },  // Ramura dreaptă
//            { glm::mat2(0.1f, 0.0f, 0.0f, 0.1f), glm::vec2(0.0f, 0.1f) }   // Frunze mici
//    };
//
//    for (int iter = 0; iter < iterations; iter++) {
//        std::vector<glm::vec3> newPoints;
//        for (const auto& point : points) {
//            for (const auto& t : transforms) {
//                glm::vec2 transformed = t.mat * glm::vec2(point.x, point.y) + t.offset;
//                newPoints.emplace_back(transformed.x, transformed.y, point.z);
//            }
//        }
//        points = newPoints; //  Înlocuim punctele vechi cu cele noi
//    }
//
//    //  Calculăm normalele folosind produsul vectorial
//    for (size_t i = 0; i < points.size(); i += 3) {
//        if (i + 2 >= points.size()) break;  // Prevenim accesul invalid la vector
//
//        glm::vec3 p1 = points[i];
//        glm::vec3 p2 = points[i + 1];
//        glm::vec3 p3 = points[i + 2];
//
//        glm::vec3 normal = glm::normalize(glm::cross(p2 - p1, p3 - p1));
//
//        //  Adăugăm vertecșii + normalele corect în `vertexData`
//        auto addVertex = [&](glm::vec3 pos, glm::vec3 norm) {
//            vertexData.push_back(pos.x);
//            vertexData.push_back(pos.y);
//            vertexData.push_back(pos.z);
//            vertexData.push_back(norm.x);
//            vertexData.push_back(norm.y);
//            vertexData.push_back(norm.z);
//        };
//
//        addVertex(p1, normal);
//        addVertex(p2, normal);
//        addVertex(p3, normal);
//    }
//
//    //  Transferăm datele către OpenGL
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);
//
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(0);
//
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
//    glEnableVertexAttribArray(1);
//
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
//}
////////////////////////////////////////////////////////////////
//void FractalRenderer::generateIFS(int iterations) {
//    vertexData.clear();
//
//    //  Set inițial de puncte (trunchiul)
//    std::vector<glm::vec3> points = { glm::vec3(0.0f, 0.0f, 0.0f) };
//
//    //  Matricile de transformare pentru copac
//    struct Transform {
//        glm::mat3 mat;
//        glm::vec3 offset;
//    };
//
//    std::vector<Transform> transforms = {
//            { glm::mat3(0.6f), glm::vec3(0.0f, 1.0f, 0.0f) },  // Trunchiul
//            { glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(0, 0, 1))), glm::vec3(0.0f, 1.0f, 0.0f) },  // Ramura stângă
//            { glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(-30.0f), glm::vec3(0, 0, 1))), glm::vec3(0.0f, 1.0f, 0.0f) } // Ramura dreaptă
//    };
//
//    for (int iter = 0; iter < iterations; iter++) {
//        std::vector<glm::vec3> newPoints;
//        for (const auto& point : points) {
//            for (const auto& t : transforms) {
//                glm::vec3 transformed = t.mat * point + t.offset;
//                newPoints.push_back(transformed);
//            }
//        }
//        points = newPoints;  //  Înlocuim punctele vechi cu cele noi
//    }
//
//    //  Salvăm punctele pentru a le folosi la generarea cilindrilor
//    branchPositions = points;
//}
void FractalRenderer::generateIFS(int iterations) {
    vertexData.clear();
    std::vector<glm::vec3> points = { glm::vec3(0.0f, 0.0f, 0.0f) };

    struct Transform {
        glm::mat3 mat;
        glm::vec3 offset;
        float probability;
    };

    std::vector<Transform> transforms = {
            { glm::mat3(0.00, 0.00, 0.0,  0.0, 0.18, 0.0,  0.0, 0.0, 0.00), glm::vec3(0.0, -0.8, 0), 0.01 },
            { glm::mat3(0.85, 0.00, 0.0,  0.0, 0.85, 0.1, -0.1, 0.85, 0.0), glm::vec3(0.0, 1.6, 0), 0.85 },
            { glm::mat3(0.20, -0.20, 0.0,  0.2, 0.20, 0.0,  0.0, 0.3, -0.2), glm::vec3(0.0, 0.6, 0), 0.07 },
            { glm::mat3(-0.20, 0.20, 0.0,  0.2, 0.20, 0.0,  0.0, 0.3, 0.2), glm::vec3(0.0, 0.6, 0), 0.07 }
    };

    std::vector<float> probSum;
    float sum = 0.0f;
    for (const auto& t : transforms) {
        sum += t.probability;
        probSum.push_back(sum);
    }

    for (int iter = 0; iter < iterations; iter++) {
        std::vector<glm::vec3> newPoints;
        for (const auto& point : points) {
            float r = static_cast<float>(rand()) / RAND_MAX * sum;
            for (size_t i = 0; i < transforms.size(); i++) {
                if (r < probSum[i]) {
                    glm::vec3 transformed = transforms[i].mat * point + transforms[i].offset;
                    newPoints.push_back(transformed);
                    break; // Aplicăm doar o singură transformare per punct
                }
            }
        }
        if (newPoints.empty()) break;
        points = newPoints;
    }

    for (const auto& point : points) {
        glm::vec3 normal = glm::vec3(0, 0, 1);
        vertexData.push_back(point.x);
        vertexData.push_back(point.y);
        vertexData.push_back(point.z);
        vertexData.push_back(normal.x);
        vertexData.push_back(normal.y);
        vertexData.push_back(normal.z);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}






void FractalRenderer::generateIFSwithmatrix(const std::vector<std::vector<float>>& ifsMatrix, int iterations) {
    vertexData.clear();
    std::vector<glm::vec3> points = { glm::vec3(0.0f, 0.0f, 0.0f) };

    struct Transform {
        glm::mat3 mat;
        glm::vec3 offset;
        float probability;
    };

    std::vector<Transform> transforms;

    for (const auto& row : ifsMatrix) {
        if (row.size() != 13) continue;
        glm::mat3 mat = {
                row[0], row[1], row[2],
                row[3], row[4], row[5],
                row[6], row[7], row[8]
        };
        glm::vec3 offset = { row[9], row[10], row[11] };
        float probability = row[12];
        transforms.push_back({ mat, offset, probability });
    }

    for (int iter = 0; iter < iterations; iter++) {
        std::vector<glm::vec3> newPoints;
        for (const auto& point : points) {
            for (const auto& t : transforms) {
                glm::vec3 transformed = t.mat * glm::vec3(point) + t.offset;
                newPoints.push_back(transformed);
            }
        }
        points = newPoints;
    }

    //  Calculăm normalele pentru fiecare triplet de puncte
    for (size_t i = 0; i < points.size(); i += 3) {
        if (i + 2 >= points.size()) break;

        glm::vec3 p1 = points[i];
        glm::vec3 p2 = points[i + 1];
        glm::vec3 p3 = points[i + 2];

        glm::vec3 normal = glm::normalize(glm::cross(p2 - p1, p3 - p1));

        auto addVertex = [&](glm::vec3 pos, glm::vec3 norm) {
            vertexData.push_back(pos.x);
            vertexData.push_back(pos.y);
            vertexData.push_back(pos.z);
            vertexData.push_back(norm.x);
            vertexData.push_back(norm.y);
            vertexData.push_back(norm.z);
        };

        addVertex(p1, normal);
        addVertex(p2, normal);
        addVertex(p3, normal);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void FractalRenderer::generateMeshFromIFS() {
    vertexData.clear();
    float radius = 0.1f;  //  Grosimea ramurilor

    for (size_t i = 0; i < branchPositions.size(); i += 2) {
        if (i + 1 >= branchPositions.size()) break;

        glm::vec3 p1 = branchPositions[i];
        glm::vec3 p2 = branchPositions[i + 1];

        glm::vec3 direction = glm::normalize(p2 - p1);
        glm::vec3 perpendicular = glm::cross(direction, glm::vec3(0, 0, 1));
        if (glm::length(perpendicular) < 0.01f) perpendicular = glm::cross(direction, glm::vec3(0, 1, 0));

        perpendicular = glm::normalize(perpendicular) * radius;

        std::vector<glm::vec3> bottomRing, topRing;
        int numSides = 8;

        for (int j = 0; j < numSides; j++) {
            float theta = j / (float)numSides * glm::pi<float>() * 2.0f;
            glm::vec3 offset = perpendicular * glm::vec3(cos(theta), sin(theta), 0);
            bottomRing.push_back(p1 + offset);
            topRing.push_back(p2 + offset);
        }

        for (int j = 0; j < numSides; j++) {
            int next = (j + 1) % numSides;

            glm::vec3 normal1 = glm::normalize(glm::cross(topRing[j] - bottomRing[j], bottomRing[next] - bottomRing[j]));
            glm::vec3 normal2 = glm::normalize(glm::cross(topRing[next] - topRing[j], bottomRing[next] - topRing[j]));

            auto addVertex = [&](glm::vec3 pos, glm::vec3 norm) {
                vertexData.push_back(pos.x);
                vertexData.push_back(pos.y);
                vertexData.push_back(pos.z);
                vertexData.push_back(norm.x);
                vertexData.push_back(norm.y);
                vertexData.push_back(norm.z);
            };

            addVertex(bottomRing[j], normal1);
            addVertex(bottomRing[next], normal1);
            addVertex(topRing[j], normal1);

            addVertex(topRing[j], normal2);
            addVertex(bottomRing[next], normal2);
            addVertex(topRing[next], normal2);
        }
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void FractalRenderer::draw() {

    glBindVertexArray(VAO);
    checkOpenGLError("Before draw");

    glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 6);  //  Fiecare vertex are 6 float-uri (3 poziții + 3 normale)
    checkOpenGLError("After draw");
}
void FractalRenderer::drawM() {

    glBindVertexArray(VAO);
    checkOpenGLError("Before draw");

    glDrawArrays(GL_TRIANGLES, 0, mountainVertexData.size() / 6);  //  Fiecare vertex are 6 float-uri (3 poziții + 3 normale)
    checkOpenGLError("After draw");
}

void FractalRenderer::generateFractalMountainSphere(int subdivisions, float radius) {
    mountainVertexData.clear();

    struct Triangle {
        glm::vec3 v0, v1, v2;
    };

    std::vector<glm::vec3> vertices;
    std::vector<Triangle> triangles;

    // 🔹 Icosahedron base (unit sphere approximation)
    float t = (1.0 + sqrt(5.0)) / 2.0;
    vertices = {
            {-1,  t,  0}, {1,  t,  0}, {-1, -t,  0}, {1, -t,  0},
            {0, -1,  t}, {0,  1,  t}, {0, -1, -t}, {0,  1, -t},
            { t,  0, -1}, { t,  0,  1}, {-t,  0, -1}, {-t,  0,  1}
    };

    for (auto& v : vertices)
        v = glm::normalize(v) * radius;

    std::vector<glm::ivec3> faces = {
            {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7,10}, {0,10,11},
            {1, 5, 9}, {5,11,4}, {11,10,2}, {10,7,6}, {7,1,8},
            {3,9,4}, {3,4,2}, {3,2,6}, {3,6,8}, {3,8,9},
            {4,9,5}, {2,4,11}, {6,2,10}, {8,6,7}, {9,8,1}
    };

    // 🔁 Subdivide
    auto midpoint_cache = std::map<std::pair<int, int>, int>();
    auto get_midpoint = [&](int a, int b) -> int {
        auto key = std::minmax(a, b);
        if (midpoint_cache.count(key)) return midpoint_cache[key];

        glm::vec3 mid = glm::normalize((vertices[a] + vertices[b]) * 0.5f) * radius;
        vertices.push_back(mid);
        int idx = vertices.size() - 1;
        midpoint_cache[key] = idx;
        return idx;
    };

    for (int i = 0; i < subdivisions; ++i) {
        std::vector<glm::ivec3> new_faces;
        for (const auto& f : faces) {
            int a = get_midpoint(f.x, f.y);
            int b = get_midpoint(f.y, f.z);
            int c = get_midpoint(f.z, f.x);
            new_faces.push_back({f.x, a, c});
            new_faces.push_back({f.y, b, a});
            new_faces.push_back({f.z, c, b});
            new_faces.push_back({a, b, c});
        }
        faces = new_faces;
    }

    // 🔄 Deform (FBM-style)
    auto fbm = [](glm::vec3 pos, int octaves, float lacunarity, float gain) {
        float amplitude = 0.1f;
        float frequency = 1.5f;
        float sum = 0.0f;
        for (int i = 0; i < octaves; ++i) {
            sum += amplitude * glm::simplex(pos * frequency);
            frequency *= lacunarity;
            amplitude *= gain;
        }
        return sum;
    };

    for (auto& v : vertices) {
        float noise = fbm(glm::normalize(v), 4, 2.0f, 0.5f);
        v += glm::normalize(v) * noise * 0.2f;
    }

    // 🔄 Vertex + normals
    for (const auto& f : faces) {
        glm::vec3 v0 = vertices[f.x];
        glm::vec3 v1 = vertices[f.y];
        glm::vec3 v2 = vertices[f.z];

        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        for (auto& v : {v0, v1, v2}) {
            mountainVertexData.push_back(v.x);
            mountainVertexData.push_back(v.y);
            mountainVertexData.push_back(v.z);
            mountainVertexData.push_back(normal.x);
            mountainVertexData.push_back(normal.y);
            mountainVertexData.push_back(normal.z);
        }
    }

    // 🔁 Upload to GPU
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mountainVertexData.size() * sizeof(float), mountainVertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
void FractalRenderer::generateFractalCubeMountain(float radius, int resolution) {
    mountainVertexData.clear();

    int faces = 6;
    float step = 2.0f / (resolution - 1);

    auto cubeToSphere = [](glm::vec3 p) {
        float x2 = p.x * p.x;
        float y2 = p.y * p.y;
        float z2 = p.z * p.z;
        return glm::vec3(
                p.x * sqrtf(1.0f - (y2 + z2) / 2.0f + (y2 * z2) / 3.0f),
                p.y * sqrtf(1.0f - (x2 + z2) / 2.0f + (x2 * z2) / 3.0f),
                p.z * sqrtf(1.0f - (x2 + y2) / 2.0f + (x2 * y2) / 3.0f)
        );
    };

    std::vector<glm::vec3> positions;

    // 6 fețe ale cubului
    std::vector<glm::vec3> faceNormals = {
            { 1, 0, 0 }, { -1, 0, 0 },
            { 0, 1, 0 }, { 0, -1, 0 },
            { 0, 0, 1 }, { 0, 0, -1 }
    };

    std::vector<glm::vec3> tangents = {
            { 0, 1, 0 }, { 0, 1, 0 },
            { 1, 0, 0 }, { 1, 0, 0 },
            { 1, 0, 0 }, { 1, 0, 0 }
    };

    for (int f = 0; f < faces; ++f) {
        glm::vec3 normal = faceNormals[f];
        glm::vec3 tangent = tangents[f];
        glm::vec3 bitangent = glm::cross(normal, tangent);

        for (int y = 0; y < resolution; ++y) {
            for (int x = 0; x < resolution; ++x) {
                glm::vec2 percent = glm::vec2(x, y) / float(resolution - 1);
                glm::vec3 pointOnFace = normal +
                                        (percent.x - 0.5f) * 2.0f * tangent +
                                        (percent.y - 0.5f) * 2.0f * bitangent;

                glm::vec3 pointOnSphere = cubeToSphere(glm::normalize(pointOnFace));

                // Fake perlin cu sin pt dealuri
                float noise = 0.05f * sin(10.0f * pointOnSphere.x) *
                              sin(10.0f * pointOnSphere.y) *
                              sin(10.0f * pointOnSphere.z);

                glm::vec3 displaced = pointOnSphere * (radius + noise);
                glm::vec3 normalVec = glm::normalize(pointOnSphere);

                mountainVertexData.push_back(displaced.x);
                mountainVertexData.push_back(displaced.y);
                mountainVertexData.push_back(displaced.z);
                mountainVertexData.push_back(normalVec.x);
                mountainVertexData.push_back(normalVec.y);
                mountainVertexData.push_back(normalVec.z);

                positions.push_back(displaced);
            }
        }
    }

    // Indici pentru triunghiuri
    std::vector<unsigned int> indices;
    for (int f = 0; f < faces; ++f) {
        int faceOffset = f * resolution * resolution;
        for (int y = 0; y < resolution - 1; ++y) {
            for (int x = 0; x < resolution - 1; ++x) {
                int i0 = faceOffset + y * resolution + x;
                int i1 = faceOffset + y * resolution + x + 1;
                int i2 = faceOffset + (y + 1) * resolution + x;
                int i3 = faceOffset + (y + 1) * resolution + x + 1;

                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i2);

                indices.push_back(i2);
                indices.push_back(i1);
                indices.push_back(i3);
            }
        }
    }

    // Upload în GPU
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mountainVertexData.size() * sizeof(float), mountainVertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
void FractalRenderer::generateSimpleSphere(float radius, int latBands, int longBands) {
    mountainVertexData.clear();

    for (int lat = 0; lat <= latBands; ++lat) {
        float theta = lat * glm::pi<float>() / latBands;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= longBands; ++lon) {
            float phi = lon * 2.0f * glm::pi<float>() / longBands;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            glm::vec3 position;
            position.x = radius * sinTheta * cosPhi;
            position.y = radius * cosTheta;
            position.z = radius * sinTheta * sinPhi;

            glm::vec3 normal = glm::normalize(position);

            mountainVertexData.push_back(position.x);
            mountainVertexData.push_back(position.y);
            mountainVertexData.push_back(position.z);
            mountainVertexData.push_back(normal.x);
            mountainVertexData.push_back(normal.y);
            mountainVertexData.push_back(normal.z);
        }
    }

    std::vector<unsigned int> indices;
    for (int lat = 0; lat < latBands; ++lat) {
        for (int lon = 0; lon < longBands; ++lon) {
            int first = (lat * (longBands + 1)) + lon;
            int second = first + longBands + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mountainVertexData.size() * sizeof(float), mountainVertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}