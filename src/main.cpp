// Add GLM include for glm::vec3 etc.
#include <unordered_map>
#include <glm/glm.hpp>
#include "../vendor/json/json.hpp"
#include <map>
#include <string>
#include <unordered_map>
// Species-specific iteration offset for demo rendering
std::unordered_map<std::string, int> speciesIterationOffset = {
    {"HighlandOak", 13}
};
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
#include <fstream>
#include <chrono>
#include <iomanip>
#include <GLFW/glfw3.h>

extern float rotationX;
extern float rotationY;

// Light rotation controls
bool rotateLight = false;
glm::vec3 lightPos = glm::vec3(10.0f, 10.0f, 10.0f);
float lightAngle = 0.0f;
bool paused = false;
bool regenerating = false;

// Forward declaration
void renderScene(Window& window, Shader& shader_tree, Shader& shader_sphere, Mesh& ground, glm::mat4& projection, float currentFrame, float deltaTime);
void initSlotGridDiagonal();
void renderSceneDiagonal(Window& window, Shader& shader_tree, Shader& shader_sphere, Mesh& ground, glm::mat4& projection, float currentFrame, float deltaTime);
void benchMode(int count, int frames, const std::string& out_csv);
void initSlotsSingleModel(const std::string& obj_path, int count, float spacing);
void renderSceneDemo(Window& window, Shader& shader_tree, Shader& shader_sphere, Mesh& ground, glm::mat4& projection, float currentFrame, float deltaTime);
void initDemoSlots();
void initHilbertSlotsWithLeaves(const std::string& jsonPath);


struct TreeSlot {
    std::vector<std::shared_ptr<Mesh>> evolutionStages;
    int currentStage = 0;
    float lastSwitchTime = 0.0f;
    glm::vec3 position;
};

std::vector<TreeSlot> treeSlots;


// GLFW key callback
// Ensure required headers are included
#include <filesystem>
#include <random>
#include <vector>
#include <string>
#include <iostream>

// Forward declaration for Hilbert initialization
void initWithHilbert(const std::string& hilbertFile);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        paused = !paused;
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        regenerating=true;
        // Pick random JSON from FillSpace folder using POSIX dirent
        std::string folder = "../lsysGrammar/FillSpace";
        std::vector<std::string> jsonFiles;

        DIR* dir = opendir(folder.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string name = entry->d_name;
                if (name.size() >= 5 && name.substr(name.size() - 5) == ".json") {
                    jsonFiles.push_back(folder + "/" + name);
                }
            }
            closedir(dir);
        }

        if (!jsonFiles.empty()) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(0, jsonFiles.size() - 1);
            std::string randomFile = jsonFiles[distrib(gen)];

            std::cout << "🔄 Regenerating Hilbert from: " << randomFile << std::endl;
            initHilbertSlotsWithLeaves(randomFile);
            regenerating=false;

        } else {
            std::cerr << "❌ No JSON files found in FillSpace folder.\n";
            regenerating=false;

        }

    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        rotateLight = !rotateLight;
    }
}
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

// Structure to hold trunk and leaf mesh pair for a stage
struct TreeWithLeaves {
    std::shared_ptr<Mesh> trunk;
    std::shared_ptr<Mesh> leaves;
    std::string speciesName;
};

struct TreeSlotWithLeaves {
    std::vector<TreeWithLeaves> evolutionStages;
    int currentStage = 0;
    float lastSwitchTime = 0.0f;
    glm::vec3 position;
    glm::vec3 scale = glm::vec3(1.0f);
    float rotationY = 0.0f;
};

// Keep the original vector for compatibility with renderSceneDemo
static std::vector<TreeSlotWithLeaves> treeSlotsDemo;

// Map with trunk/leaf colors for each species
std::map<std::string, std::pair<glm::vec3, glm::vec3>> speciesColors = {
    {"Demo/SpiralGuardian",      {{0.45f, 0.25f, 0.1f},  {1.0f, 0.6f, 0.8f}}},
    {"Demo/EntropicTitan",       {{0.3f,  0.2f, 0.1f},   {0.6f, 1.0f, 0.6f}}},
    {"Demo/HighlandOak",         {{0.4f,  0.2f, 0.05f},  {0.3f, 0.8f, 0.2f}}},
    {"Demo/GoldenPineTree",      {{0.35f, 0.22f, 0.05f}, {0.1f, 0.5f, 0.1f}}},
    {"Demo/RandomDream",         {{0.3f,  0.15f, 0.1f},  {0.9f, 0.9f, 0.3f}}},
    {"Demo/ThunderstruckGiant",  {{0.25f, 0.2f, 0.1f},   {0.7f, 0.5f, 1.0f}}},
    {"Demo/GoldenOaktree",       {{0.5f,  0.35f, 0.1f},  {0.9f, 0.75f, 0.2f}}}
};
//std::map<std::string, std::pair<glm::vec3, glm::vec3>> speciesColors = {
//        {"SpiralGuardian",      {{0.45f, 0.25f, 0.1f},  {1.0f, 0.6f, 0.8f}}},
//        {"EntropicTitan",       {{0.3f,  0.2f, 0.1f},   {0.6f, 1.0f, 0.6f}}},
//        {"HighlandOak",         {{0.4f,  0.2f, 0.05f},  {0.3f, 0.8f, 0.2f}}},
//        {"GoldenPineTree",      {{0.35f, 0.22f, 0.05f}, {0.1f, 0.5f, 0.1f}}},
//        {"RandomDream",         {{0.3f,  0.15f, 0.1f},  {0.9f, 0.9f, 0.3f}}},
//        {"ThunderstruckGiant",  {{0.25f, 0.2f, 0.1f},   {0.7f, 0.5f, 1.0f}}},
//        {"GoldenOaktree",       {{0.5f,  0.35f, 0.1f},  {0.9f, 0.75f, 0.2f}}}
//};

// Helper for grid position
static glm::vec3 computeSlotPosition(int speciesIdx, int slotIdx) {
    int cols = 7;
    int rows = 4;
    float spacingX = 6.0f;
    float spacingZ = 10.0f;
    float x = (speciesIdx - cols / 2.0f) * spacingX;
    float z = (slotIdx - rows / 2.0f) * spacingZ;
    return glm::vec3(x, 0.0f, z);
}

//void initDemoSlots() {
//    treeSlotsDemo.clear();
//
//    std::vector<std::string> demoSpeciesNames = {
//        "GoldenOaktree__parametric_", "GoldenPineTree__parametric_",
//        "EntropicTitan__stochastic_", "RandomDream__stochastic_", "HighlandOak__parametric_",
//        "SpiralGuardian__parametric_", "ThunderstruckGiant__parametric_"
//    };
//
//    // For each species, load all 7 stages (iter_1 to iter_7, possibly offset), then create 4 slots per species
//    float zSpacing = 8.0f;
//    for (int i = 0; i < (int)demoSpeciesNames.size(); ++i) {
//        const std::string& name = demoSpeciesNames[i];
//        int offset = 0;
//        // Remove trailing "__parametric_" or "__stochastic_" for offset lookup
//        std::string baseName = name;
//        size_t pos = baseName.find("__parametric_");
//        if (pos != std::string::npos) baseName = baseName.substr(0, pos);
//        pos = baseName.find("__stochastic_");
//        if (pos != std::string::npos) baseName = baseName.substr(0, pos);
//        if (speciesIterationOffset.find(baseName) != speciesIterationOffset.end()) {
//            offset = speciesIterationOffset[baseName];
//        }
//        // Extract speciesName for color mapping (remove suffixes)
//        std::string speciesName = name;
//        size_t speciesPos = speciesName.find("__parametric_");
//        if (speciesPos != std::string::npos) speciesName = speciesName.substr(0, speciesPos);
//        speciesPos = speciesName.find("__stochastic_");
//        if (speciesPos != std::string::npos) speciesName = speciesName.substr(0, speciesPos);
//
//        // Load all 7 stages, iter_1 to iter_7 (or offsetted)
//        std::vector<TreeWithLeaves> evolutionStages;
//        for (int k = 1; k <= 7; ++k) {
//            std::string iterStr = std::to_string(k + offset);
//            std::string meshName = name + "_iter_" + iterStr + ".obj";
//            std::string leafName = name + "_iter_" + iterStr + "_leaf.obj";
//
//            TreeWithLeaves stage;
//            stage.trunk = std::make_shared<Mesh>();
//            stage.leaves = std::make_shared<Mesh>();
//            stage.speciesName = speciesName;
//            bool trunkLoaded = stage.trunk->loadFromOBJWithNormalsDebug("../lsysGrammar/Demo/" + meshName);
//            bool leafLoaded = stage.leaves->loadFromOBJWithNormalsDebug("../lsysGrammar/Demo/" + leafName);
//            if (trunkLoaded /*&& leafLoaded*/) {
//                evolutionStages.push_back(stage);
//            }
//        }
//        // Ensure order is ascending by k (already is)
//        // For the 4 demo slots, assign all stages but start at iter_4..iter_7 (indices 3..6)
//        for (int j = 0; j < 4; ++j) {
//            if (evolutionStages.size() < 7) continue; // skip incomplete species
//            TreeSlotWithLeaves slot;
//            slot.evolutionStages = evolutionStages;
//            slot.currentStage = j + 3; // 3,4,5,6 (iter_4 to iter_7)
//            slot.lastSwitchTime = 0.0f;
//            // Layout: 4 slots per species along Z axis, closest (iter_4) at z=0, then further back
//            float xOffset = (i - demoSpeciesNames.size()/2.0f) * 9.0f;
//            slot.position = glm::vec3(xOffset, 0.0f, -j * zSpacing);
//            slot.scale = glm::vec3(5.0f);
//            slot.rotationY = 0.0f;
//            treeSlotsDemo.push_back(slot);
//        }
//    }
//}
void initDemoSlots() {
    treeSlotsDemo.clear();

    std::vector<std::string> demoSpeciesNames = {
            "GoldenOaktree__parametric_", "GoldenPineTree__parametric_",
            "EntropicTitan__stochastic_", "RandomDream__stochastic_", "HighlandOak__parametric_",
            "SpiralGuardian__parametric_", "ThunderstruckGiant__parametric_"
    };

    // For each species, fill 4 slots in the grid
    for (int i = 0; i < (int)demoSpeciesNames.size(); ++i) {
        const std::string& name = demoSpeciesNames[i];
        for (int slotIdx = 0; slotIdx < 4; ++slotIdx) {
            TreeSlotWithLeaves slot;
            slot.position = computeSlotPosition(i, slotIdx);

            int offset = 0;
            // Remove trailing "__parametric_" or "__stochastic_" for offset lookup
            std::string baseName = name;
            size_t pos = baseName.find("__parametric_");
            if (pos != std::string::npos) baseName = baseName.substr(0, pos);
            pos = baseName.find("__stochastic_");
            if (pos != std::string::npos) baseName = baseName.substr(0, pos);
            if (speciesIterationOffset.find(baseName) != speciesIterationOffset.end()) {
                offset = speciesIterationOffset[baseName];
            }
            for (int k = 4; k <= 7; ++k) {
                std::string iterStr = std::to_string(k + offset);
                std::string meshName = name + "_iter_" + iterStr + ".obj";
                std::string leafName = name + "_iter_" + iterStr + "_leaf.obj";

                TreeWithLeaves stage;
                stage.trunk = std::make_shared<Mesh>();
                stage.leaves = std::make_shared<Mesh>();
                stage.speciesName = baseName;
                bool trunkLoaded = stage.trunk->loadFromOBJWithNormalsDebug("../lsysGrammar/Demo/" + meshName);
                bool leafLoaded = stage.leaves->loadFromOBJWithNormalsDebug("../lsysGrammar/Demo/" + leafName);
                if (trunkLoaded /*&& leafLoaded*/) {
                    slot.evolutionStages.push_back(stage);
                }
            }
            if (!slot.evolutionStages.empty()) {
                slot.currentStage = 0;
                slot.lastSwitchTime = 0.0f;
                treeSlotsDemo.push_back(slot);
            }
        }
    }
}
// New function: load slots with leaves from Hilbert curve JSON
void initHilbertSlotsWithLeaves(const std::string& jsonPath) {
    treeSlotsDemo.clear();
    std::ifstream inFile(jsonPath);
    if (!inFile.is_open()) {
        std::cerr << "❌ Cannot open JSON: " << jsonPath << "\n";
        return;
    }
    nlohmann::json layout;
    inFile >> layout;

    for (const auto& item : layout) {
        std::string meshName = item["mesh"].get<std::string>();
        if (meshName.find("leaf") != std::string::npos) continue;

        TreeSlotWithLeaves slot;
        // Apply centering offset so the full grid fits symmetrically in [-25, 25] if spacing is ~7.1 and grid is 7x7
        float spacing = 7.1f;
        float gridSize = 7.0f;
        float gridOffset = (gridSize - 1) * spacing / 2.0f;
        float x = item["position"][0].get<float>() - gridOffset;
        float y = item["position"][1].get<float>();
        float z = item["position"][2].get<float>() - gridOffset;
        slot.position = glm::vec3(x, y, z);
        if (item.contains("scale")) {
            float s = item["scale"].get<float>();
            slot.scale = glm::vec3(s);
        }
        if (item.contains("rotation_y")) {
            slot.rotationY = glm::radians(item["rotation_y"].get<float>());
        }
        slot.currentStage = 0;
        slot.lastSwitchTime = 0.0f;

        std::string fullPath = "../lsysGrammar/" + meshName;
        std::string leafPath = fullPath.substr(0, fullPath.size() - 4) + "_leaf.obj";

        // Determine base name and offset
        size_t specEnd = meshName.find("__");
        std::string baseName = (specEnd != std::string::npos) ? meshName.substr(0, specEnd) : "Unknown";
        int offset = (baseName == "Demo/HighlandOak") ? 13 : 0;

        std::string speciesName = baseName;

        // Extract iter
        std::string iterStr = "_iter_";
        size_t iterPos = meshName.find(iterStr);
        int iter = 1;
        if (iterPos != std::string::npos) {
            iter = std::stoi(meshName.substr(iterPos + iterStr.length()));
        }

        // Load all 7 iterations
        std::vector<TreeWithLeaves> evolutionStages;
        for (int k = 1; k <= 7; ++k) {
            int iterIndex = k + offset;
            std::string stem = meshName.substr(0, iterPos);
            std::string meshIter = stem + "_iter_" + std::to_string(iterIndex) + ".obj";
            std::string leafIter = stem + "_iter_" + std::to_string(iterIndex) + "_leaf.obj";

            TreeWithLeaves stage;
            stage.trunk = std::make_shared<Mesh>();
            stage.leaves = std::make_shared<Mesh>();
            stage.speciesName = speciesName;

            bool trunkLoaded = stage.trunk->loadFromOBJWithNormalsDebug("../lsysGrammar/" + meshIter);
            bool leafLoaded = stage.leaves->loadFromOBJWithNormalsDebug("../lsysGrammar/" + leafIter);
            if (trunkLoaded) {
                evolutionStages.push_back(stage);
            }
        }

        if (evolutionStages.size() == 7) {
            slot.evolutionStages = evolutionStages;
            treeSlotsDemo.push_back(slot);
        }
    }
}

// New function: load slots from Hilbert curve JSON
void initSlotsFromHilbertJSON(const std::string& jsonPath) {
    treeSlots.clear();
    std::ifstream inFile(jsonPath);
    if (!inFile.is_open()) {
        std::cerr << "❌ Cannot open JSON: " << jsonPath << "\n";
        return;
    }
    nlohmann::json layout;
    inFile >> layout;
    for (const auto& item : layout) {
        TreeSlot slot;
        slot.position = glm::vec3(
            item["position"][0].get<float>(),
            item["position"][1].get<float>(),
            item["position"][2].get<float>()
        );
        slot.currentStage = 0;
        slot.lastSwitchTime = 0.0f;

        auto mesh = std::make_shared<Mesh>();
        std::string path = "../lsysGrammar/" + item["mesh"].get<std::string>();
        if (mesh->loadFromOBJWithNormalsDebug(path)) {
            slot.evolutionStages.push_back(mesh);
            treeSlots.push_back(slot);
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
    // Set GLFW key callback for controls
    glfwSetKeyCallback(window.window, key_callback);
    //initSlotGridDiagonal();
//    initDemoSlots();
    initHilbertSlotsWithLeaves("../lsysGrammar/FillSpace/scene_layout_hilbert.json");
//    int rows = 4, cols = 5;
//    float spacing = 10.0f;
//    std::default_random_engine rng(std::random_device{}());
//
//    for (int i = 0; i < rows; ++i) {
//        for (int j = 0; j < cols; ++j) {
//            TreeSlot slot;
//            slot.position = glm::vec3(j * spacing - cols * spacing / 2.0f, 0.0f, i * spacing - rows * spacing / 2.0f);
//
//            std::string baseName = randomTreeBaseName(rng);
//            int offset = 0;
//            if (baseName.find("FernParametric__parametric_") != std::string::npos) {
//                offset = 7;
//            }
//            for (int k = 1; k <= 6; ++k) {
//                auto mesh = std::make_shared<Mesh>();
//                std::string path = "../lsysGrammar/Catalog_opt/" + baseName + "_iter_" + std::to_string(k+offset) + ".obj";
//                if (mesh->loadFromOBJWithNormalsDebug(path)) {
//                    slot.evolutionStages.push_back(mesh);
//                }
//            }
//
//            slot.currentStage = rand() % slot.evolutionStages.size();
//            slot.lastSwitchTime = glfwGetTime();
//            treeSlots.push_back(slot);
//        }
//    }

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1600.0f / 900.0f, 0.1f, 100.0f);
    float lastFrame = 0.0f;

    while (!window.shouldClose()) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Light rotation logic
        if (rotateLight) {
            lightAngle += 0.3*deltaTime;

            // Rotație verticală (soare care răsare și apune pe cer)
            float radius = 35.0f;
            lightPos.x = 0.0f; // centru sus pe planșă
            lightPos.y = radius * sin(lightAngle);  // ↑ și ↓ pe cer
            lightPos.z = radius * cos(lightAngle);  // avans în z (spre scenă)
        }
//        renderScene(window, shader_tree, shader_sphere, ground, projection, currentFrame, deltaTime);
//        renderSceneDiagonal(window, shader_tree, shader_sphere, ground, projection, currentFrame, deltaTime);
        if(!regenerating){
            renderSceneDemo(window, shader_tree, shader_sphere, ground, projection, currentFrame, deltaTime);

        }
    }
    return 0;
}
// Înainte de benchMode, sau într-un header vizibil:
void initSlotsSingleModel(const std::string& obj_path, int count, float spacing = 10.0f) {
    treeSlots.clear();
    // Încarc o singură dată mesh-ul „master”
    auto master = std::make_shared<Mesh>();
    if (!master->loadFromOBJWithNormalsDebug(obj_path)) {
        std::cerr << "❌ Fail loading model for benchmarking: " << obj_path << "\n";
        std::exit(1);
    }

    // Aşez „count” instanţe ale lui master în grid liniar
    for (int i = 0; i < count; ++i) {
        TreeSlot slot;
        slot.evolutionStages.push_back(master);
        slot.currentStage  = 0;
        // poziţia pe axa X, cu spacing
        slot.position = glm::vec3((i - count/2.0f) * spacing, 0.0f, 0.0f);
        treeSlots.push_back(slot);
    }
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




void benchMode(int count, int frames, const std::string& out_csv) {
    // 1) Initialize GLFW + headless window
    if (!glfwInit()) {
        std::cerr << "❌ glfwInit failed\n";
        return;
    }
    // create invisible context
    Window window(640, 480, "Benchmark");

    // 2) Load shaders
    Shader shader_tree("../shaders/vertex-fractal.glsl",
                       "../shaders/fragment-copac.glsl");
    Shader shader_sphere("../shaders/vertex-sfera.glsl",
                         "../shaders/fragment-sfera.glsl");

    // 3) Load ground mesh (already sets up VAO/VBO in load)
    Mesh ground;
    if (!ground.loadFromOBJWithNormalsDebug(
                "../lsysGrammar/precomputed_lsystems_old/plane50.obj")) {
        std::cerr << "❌ Fail loading ground mesh\n";
        glfwTerminate();
        return;
    }

    // 4) Prepare scene: a single model instantiated `count` times
    const std::string modelPath = "../lsysGrammar/Catalog/Tree__standard__iter_5.obj";
    initSlotsSingleModel(modelPath, count, /*spacing=*/5.0f);

    // 5) Projection matrix
    glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            640.0f / 480.0f,
            0.1f, 100.0f
    );

    // 6) Open CSV
    std::ofstream csv(out_csv);
    if (!csv) {
        std::cerr << "❌ Cannot open CSV: " << out_csv << "\n";
        glfwTerminate();
        return;
    }
    csv << "frame,dt_ms,fps\n";

    // 7) Measure CPU time at start
    rusage usage_start{};
    getrusage(RUSAGE_SELF, &usage_start);

    double sum_dt = 0.0;
    for (int f = 0; f < frames; ++f) {
        auto t0 = std::chrono::high_resolution_clock::now();

        // render one frame
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderSceneDiagonal(window,
                            shader_tree,
                            shader_sphere,
                            ground,
                            projection,
                            static_cast<float>(f),
                            0.0f);
        glFinish();

        auto t1 = std::chrono::high_resolution_clock::now();
        double dt = std::chrono::duration<double, std::milli>(t1 - t0).count();
        sum_dt += dt;
        double fps = (dt > 0.0) ? (1000.0 / dt) : 0.0;

        csv << f << ","
            << std::fixed << std::setprecision(3) << dt << ","
            << std::fixed << std::setprecision(1) << fps
            << "\n";
    }

    // 8) Measure CPU time at end
    rusage usage_end{};
    getrusage(RUSAGE_SELF, &usage_end);

    double user_sec = (usage_end.ru_utime.tv_sec  - usage_start.ru_utime.tv_sec)
                      + (usage_end.ru_utime.tv_usec - usage_start.ru_utime.tv_usec) / 1e6;
    double sys_sec  = (usage_end.ru_stime.tv_sec  - usage_start.ru_stime.tv_sec)
                     + (usage_end.ru_stime.tv_usec - usage_start.ru_stime.tv_usec) / 1e6;
    long   peak_rss_kb = usage_end.ru_maxrss;  // kilobytes

    // 9) Write summary line
    double avg_dt  = sum_dt / frames;
    double avg_fps = (avg_dt > 0.0) ? (1000.0 / avg_dt) : 0.0;
    double cpu_sec = user_sec + sys_sec;

    csv << "summary,"
        << std::fixed << std::setprecision(3) << avg_dt << ","
        << std::fixed << std::setprecision(1) << avg_fps << "\n"
        << "cpu_time," << std::fixed << std::setprecision(3) << cpu_sec << ",\n"
        << "peak_rss_kb," << peak_rss_kb << ",\n";

    csv.close();
    glfwTerminate();

    // 10) Print high-level summary
    std::cout << "👉 Benchmark (" << count << " instances, " << frames
              << " frames): avg FPS=" << std::fixed << std::setprecision(1)
              << avg_fps << ", CPU=" << std::fixed << std::setprecision(3)
              << cpu_sec << "s, peak RSS=" << peak_rss_kb << "KB\n";
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



void renderSceneDemo(Window& window, Shader& shader_tree, Shader& shader_sphere, Mesh& ground, glm::mat4& projection, float currentFrame, float deltaTime) {
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
    glUniform3f(glGetUniformLocation(shader_sphere.ID, "objectColor"),
                0.3f, 0.8f, 0.2f);
    glUniform3f(glGetUniformLocation(shader_tree.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(glGetUniformLocation(shader_tree.ID, "lightColor"), 0.7f, 0.7f, 0.7f);
    ground.draw(shader_sphere);

    shader_tree.use();
    glm::mat4 view = window.camera.GetViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shader_tree.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader_tree.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(shader_tree.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(glGetUniformLocation(shader_tree.ID, "lightColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(shader_tree.ID, "viewPos"),
                window.camera.Position.x,
                window.camera.Position.y,
                window.camera.Position.z);

    // Animate currentStage if not paused
    for (auto& slot : treeSlotsDemo) {
        if (!paused) {
            if (currentFrame - slot.lastSwitchTime > 1.5f) {
                slot.currentStage = (slot.currentStage + 1) % slot.evolutionStages.size();
                slot.lastSwitchTime = currentFrame;
            }
        }
        if (!slot.evolutionStages.empty()) {
            auto& pair = slot.evolutionStages[slot.currentStage];
            // Determină specia pe baza numelui fișierului trunchi
            std::string speciesName = pair.speciesName;
            glm::vec3 trunkColor = speciesColors[speciesName].first;
            glm::vec3 leafColor  = speciesColors[speciesName].second;

            glm::mat4 modelTree = glm::translate(glm::mat4(1.0f), slot.position);
            modelTree = glm::rotate(modelTree, slot.rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
            float iterBasedScale = 0.71f * (slot.currentStage + 1); // iter_1 → 1, iter_7 → 7
            modelTree = glm::scale(modelTree, slot.scale * iterBasedScale);
            //            modelTree = glm::scale(modelTree, slot.scale);
            glUniformMatrix4fv(glGetUniformLocation(shader_tree.ID, "model"), 1, GL_FALSE, glm::value_ptr(modelTree));

            // Set color for trunk and draw
            glUniform3f(glGetUniformLocation(shader_tree.ID, "objectColor"),
                        trunkColor.r, trunkColor.g, trunkColor.b);
            if (pair.trunk) pair.trunk->draw(shader_tree);
            // Set color for leaves and draw
            glUniform3f(glGetUniformLocation(shader_tree.ID, "objectColor"),
                        leafColor.r, leafColor.g, leafColor.b);
            if (pair.leaves) pair.leaves->draw(shader_tree);
        }
    }

    window.update();
}
