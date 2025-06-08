#include "core/Window.h"
#include "core/Shader.h"
#include "core/Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <random>
#include <dirent.h>
#include <sys/stat.h>
#include <memory>
#include <map>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>
#include <iomanip>
extern float rotationX;
extern float rotationY;

// Forward declaration
void renderScene(Window& window, Shader& shader_tree, Shader& shader_sphere, Mesh& ground, glm::mat4& projection, float currentFrame, float deltaTime);
void initSlotGridDiagonal();
void renderSceneDiagonal(Window& window, Shader& shader_tree, Shader& shader_sphere, Mesh& ground, glm::mat4& projection, float currentFrame, float deltaTime);
void benchMode(int count, int frames, const std::string& out_csv);

struct TreeSlot {
    std::vector<std::shared_ptr<Mesh>> evolutionStages;
    int currentStage = 0;
    float lastSwitchTime = 0.0f;
    glm::vec3 position;
};

std::vector<TreeSlot> treeSlots;

std::string randomTreeBaseName(std::default_random_engine& rng) {
    std::vector<std::string> species = {"FernParametric__parametric_", "SimpleTree__parametric_", "StochasticTree__stochastic_", "Tree__standard_"};
    std::uniform_int_distribution<size_t> dist(0, species.size() - 1);
    return species[dist(rng)];
//    return "FernParametric__parametric_";
}

void initSlotGridDiagonal() {
    treeSlots.clear();

    std::vector<std::string> species = {"FernParametric__parametric_", "SimpleTree__parametric_", "StochasticTree__stochastic_", "Tree__standard_"};
//    std::vector<std::string> species = {"TreeA", "TreeB", "TreeC", "TreeD"};
    int slotsPerSpecies = 6;
    int gridSize = 6; // 6x6 grid
    float spacing = 10.0f;

    for (int s = 0; s < (int)species.size(); ++s) {
        int offset = 0;
        if (species[s].find("FernParametric__parametric_") != std::string::npos) {
            offset = 7;
        }
        // Calculate sector start positions for each species in the grid
        // Divide grid into 4 sectors: top-left, top-right, bottom-left, bottom-right
        int sectorRow = s / 2;
        int sectorCol = s % 2;
        int sectorSize = gridSize / 2;

        int count = 0;
        for (int i = 0; i < sectorSize && count < slotsPerSpecies; ++i) {
            for (int j = 0; j < sectorSize && count < slotsPerSpecies; ++j) {
                TreeSlot slot;
                slot.position = glm::vec3(
                    (sectorCol * sectorSize + j) * spacing - gridSize * spacing / 2.0f,
                    0.0f,
                    (sectorRow * sectorSize + i) * spacing - gridSize * spacing / 2.0f
                );

                for (int k = 1; k <= 6; ++k) {
                    auto mesh = std::make_shared<Mesh>();
                    std::string path = "../lsysGrammar/Catalog/" + species[s] + "_iter_" + std::to_string(k+offset) + ".obj";
                    if (mesh->loadFromOBJWithNormalsDebug(path)) {
                        slot.evolutionStages.push_back(mesh);
                    }
                }

                if (!slot.evolutionStages.empty()) {
                    slot.currentStage = 0;
                    slot.lastSwitchTime = 0.0f;
                    treeSlots.push_back(slot);
                }
                count++;
            }
        }
    }
}

int main(int argc, char** argv) {
    // --- Benchmark mode parsing ---
    bool benchmark = false;
    int bench_count = 0, bench_frames = 0;
    std::string bench_out;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--benchmark") == 0 && i + 3 < argc) {
            benchmark = true;
            bench_count = std::atoi(argv[++i]);
            bench_frames = std::atoi(argv[++i]);
            bench_out    = argv[++i];
        }
    }
    if (benchmark) {
        benchMode(bench_count, bench_frames, bench_out);
        return 0;
    }

    Window window(1600, 900, "Fractal Evolution");
    Shader shader_tree("../shaders/vertex-fractal.glsl", "../shaders/fragment-copac.glsl");
    Shader shader_sphere("../shaders/vertex-sfera.glsl", "../shaders/fragment-sfera.glsl");
    Mesh ground;
    ground.loadFromOBJWithNormalsDebug("../lsysGrammar/precomputed_lsystems_old/plane50.obj");

    initSlotGridDiagonal();

    int rows = 4, cols = 5;
    float spacing = 10.0f;
    std::default_random_engine rng(std::random_device{}());

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            TreeSlot slot;
            slot.position = glm::vec3(j * spacing - cols * spacing / 2.0f, 0.0f, i * spacing - rows * spacing / 2.0f);

            std::string baseName = randomTreeBaseName(rng);
            int offset = 0;
            if (baseName.find("FernParametric__parametric_") != std::string::npos) {
                offset = 7;
            }
            for (int k = 1; k <= 6; ++k) {
                auto mesh = std::make_shared<Mesh>();
                std::string path = "../lsysGrammar/Catalog_opt/" + baseName + "_iter_" + std::to_string(k+offset) + ".obj";
                if (mesh->loadFromOBJWithNormalsDebug(path)) {
                    slot.evolutionStages.push_back(mesh);
                }
            }

            slot.currentStage = rand() % slot.evolutionStages.size();
            slot.lastSwitchTime = glfwGetTime();
            treeSlots.push_back(slot);
        }
    }

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1600.0f / 900.0f, 0.1f, 100.0f);
    float lastFrame = 0.0f;

    while (!window.shouldClose()) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
//        renderScene(window, shader_tree, shader_sphere, ground, projection, currentFrame, deltaTime);
        renderSceneDiagonal(window, shader_tree, shader_sphere, ground, projection, currentFrame, deltaTime);
    }
    return 0;
}

void renderScene(Window& window, Shader& shader_tree, Shader& shader_sphere, Mesh& ground, glm::mat4& projection, float currentFrame, float deltaTime) {
    window.processInput(deltaTime);
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    shader_sphere.use();

    // Render ground
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader_sphere.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    ground.draw(shader_sphere);

    shader_tree.use();
    glm::mat4 view = window.camera.GetViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shader_tree.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader_tree.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(shader_tree.ID, "lightPos"), 10.0f, 10.0f, 10.0f);
    glUniform3f(glGetUniformLocation(shader_tree.ID, "lightColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(shader_tree.ID, "viewPos"),
                window.camera.Position.x,
                window.camera.Position.y,
                window.camera.Position.z);

    for (auto& slot : treeSlots) {
        if (currentFrame - slot.lastSwitchTime > 0.3f) {
            slot.currentStage = (slot.currentStage + 1) % slot.evolutionStages.size();
            slot.lastSwitchTime = currentFrame;
        }

        glm::mat4 modelTree = glm::translate(glm::mat4(1.0f), slot.position);
        glUniformMatrix4fv(glGetUniformLocation(shader_tree.ID, "model"), 1, GL_FALSE, glm::value_ptr(modelTree));
        slot.evolutionStages[slot.currentStage]->draw(shader_tree);
    }

    window.update();
}

#include <fstream>
#include <chrono>
#include <iomanip>

void benchMode(int count, int frames, const std::string& out_csv) {
    // Initialize GLFW + window (visible for a valid context)
    if (!glfwInit()) {
        std::cerr << "❌ glfwInit failed\n";
        return;
    }
    Window window(640, 480, "Benchmark");
    // Load shaders exactly as in demo
    Shader shader_tree("../shaders/vertex-fractal.glsl", "../shaders/fragment-copac.glsl");
    Shader shader_sphere("../shaders/vertex-sfera.glsl", "../shaders/fragment-sfera.glsl");
    // Load ground mesh
    Mesh ground;
    if (!ground.loadFromOBJWithNormalsDebug("../lsysGrammar/precomputed_lsystems_old/plane50.obj")) {
        std::cerr << "❌ Fail loading ground mesh\n";
        return;
    }
    ground.setupMesh();

    // Initialize tree slots
    initSlotGridDiagonal();

    // Prepare projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                            640.0f / 480.0f,
                                            0.1f, 100.0f);

    // Open CSV file
    std::ofstream csv(out_csv);
    if (!csv) {
        std::cerr << "❌ Cannot open CSV: " << out_csv << std::endl;
        return;
    }
    csv << "frame,dt_ms\n";

    // Benchmark loop
    for (int f = 0; f < frames; ++f) {
        auto t0 = std::chrono::high_resolution_clock::now();

        // Render one frame: clear + draw all slots
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderSceneDiagonal(window, shader_tree, shader_sphere,
                            ground, projection,
                            /*currentFrame=*/static_cast<float>(f),
                            /*deltaTime=*/0.0f);

        // Ensure GPU work is done
        glFinish();

        auto t1 = std::chrono::high_resolution_clock::now();
        double dt = std::chrono::duration<double, std::milli>(t1 - t0).count();
        csv << f << "," << std::fixed << std::setprecision(3) << dt << "\n";
    }
    csv.close();

    // Cleanup
    glfwTerminate();
}

void renderSceneDiagonal(Window& window, Shader& shader_tree, Shader& shader_sphere, Mesh& ground, glm::mat4& projection, float currentFrame, float deltaTime) {
    window.processInput(deltaTime);
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    shader_sphere.use();
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view1 = window.camera.GetViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shader_sphere.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader_sphere.ID, "view"), 1, GL_FALSE, glm::value_ptr(view1));
    glUniformMatrix4fv(glGetUniformLocation(shader_sphere.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    ground.draw(shader_sphere);

    shader_tree.use();
    glm::mat4 view = window.camera.GetViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shader_tree.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader_tree.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(shader_tree.ID, "lightPos"), 10.0f, 10.0f, 10.0f);
    glUniform3f(glGetUniformLocation(shader_tree.ID, "lightColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(shader_tree.ID, "viewPos"),
                window.camera.Position.x,
                window.camera.Position.y,
                window.camera.Position.z);

    // 🔁 Matrice de rotație globală a scenei
    glm::mat4 sceneRotationMatrix = glm::rotate(glm::mat4(1.0f), rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
    sceneRotationMatrix = glm::rotate(sceneRotationMatrix, rotationX, glm::vec3(1.0f, 0.0f, 0.0f));

    for (auto& slot : treeSlots) {
        if (currentFrame - slot.lastSwitchTime > 1.5f) {
            slot.currentStage = (slot.currentStage + 1) % slot.evolutionStages.size();
            slot.lastSwitchTime = currentFrame;
        }

        glm::mat4 modelTree = sceneRotationMatrix * glm::translate(glm::mat4(1.0f), slot.position);
        glUniformMatrix4fv(glGetUniformLocation(shader_tree.ID, "model"), 1, GL_FALSE, glm::value_ptr(modelTree));
        slot.evolutionStages[slot.currentStage]->draw(shader_tree);
    }

    window.update();
}
