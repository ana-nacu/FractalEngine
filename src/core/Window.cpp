#include "Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unistd.h>

static float lastX = 400, lastY = 300;
static bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Inversăm deoarece OpenGL are originea în stânga jos
    lastX = xpos;
    lastY = ypos;

    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        win->camera.ProcessMouseMovement(xoffset, yoffset);
    }
}
Window::Window(int width, int height, const char* title) {
    if (!glfwInit()) {
        std::cerr << "Eroare: GLFW nu a putut fi inițializat!" << std::endl;
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Eroare: Nu s-a putut crea fereastra GLFW!" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Eroare: GLAD nu a fost încărcat corect!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Ascunde cursorul și îl blochează în fereastră
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetWindowUserPointer(window, this);
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::update() {
    glfwSwapBuffers(window);
    glfwPollEvents();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}
void Window::processInput(float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        level++;

std::cout << "LEVEL⬆️: " << level << std::endl;
sleep(1);
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        if(level>1)
        {level--;}
        else
        {level=1;}
        std::cout << "LEVEL⬇️: " << level << std::endl;
        sleep(1);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if(level>1)
        {level--;}
        else
        {level=1;}
        std::cout << "LEVEL⬇️: " << level << std::endl;
        sleep(1);
    }

}
void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}