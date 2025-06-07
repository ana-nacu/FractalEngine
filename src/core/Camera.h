#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Tipurile de mișcare ale camerei
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,     // Q
    DOWN    // E
};

class Camera {
public:
    // Atribute publice
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Unghiuri Euler
    float Yaw;
    float Pitch;

    // Setări
    float MovementSpeed = 2.5f;
    float MouseSensitivity = 0.1f;

    // Constructor
    Camera(glm::vec3 position);

    // Metode publice
    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
};