#pragma once
#include "Camera.h"  // ✅ Adăugăm Camera

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    void update();
    bool shouldClose() const;
    void processInput(float deltaTime);// ✅ Noua metodă pentru input
    Camera camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f));  // ✅ Inițializăm camera
    int level=5;
    float select=0;
    float pos=0;



private:
    GLFWwindow* window;
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};