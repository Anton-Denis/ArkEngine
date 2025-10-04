//
// Created by Anton on 04.07.2025.
//
#include "../../include/core/Camera.hpp"
#include "../../include/core/InputSystem.hpp"
#include "GLFW/glfw3.h"

Camera::Camera() {
    position = glm::vec3(0.0f, 2.0f, 3.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    front = glm::normalize(target - position);
    up = glm::vec3(0.0f, 1.0f, 0.0f);

    yaw = -90.0f;
    pitch = 0.0f;
    UpdateViewMatrix();
}

void Camera::UpdateViewMatrix() {
    view = glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::GetViewMatrix() const {
    return view;
}

glm::mat4 Camera::GetProjectionMatrix(float aspect) const {
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}

// ==================== New Input System Methods ====================

void Camera::Update(InputSystem* input, float deltaTime) {
    if (!m_InputEnabled || !input || !input->IsCameraInputEnabled()) return;

    ProcessKeyboardInput(input, deltaTime);
    ProcessMouseInput(input);
    ProcessGamepadInput(input, deltaTime);

    UpdateViewMatrix();
}

void Camera::ProcessKeyboardInput(InputSystem* input, float deltaTime) {
    glm::vec3 movement(0.0f);
    const float cameraSpeed = movementSpeed * deltaTime;

    if (input->IsKeyPressed(GLFW_KEY_W))
        movement += front;
    if (input->IsKeyPressed(GLFW_KEY_S))
        movement -= front;
    if (input->IsKeyPressed(GLFW_KEY_A))
        movement -= glm::normalize(glm::cross(front, up));
    if (input->IsKeyPressed(GLFW_KEY_D))
        movement += glm::normalize(glm::cross(front, up));
    if (input->IsKeyPressed(GLFW_KEY_Q))
        movement += up;
    if (input->IsKeyPressed(GLFW_KEY_E))
        movement -= up;

    // Sprint with Shift
    float speedMultiplier = 1.0f;
    if (input->IsKeyPressed(GLFW_KEY_LEFT_SHIFT) || input->IsKeyPressed(GLFW_KEY_RIGHT_SHIFT))
        speedMultiplier = 2.0f;

    if (glm::length(movement) > 0.0f)
        position += glm::normalize(movement) * cameraSpeed * speedMultiplier;
}

void Camera::ProcessMouseInput(InputSystem* input) {
    glm::vec2 mouseDelta = input->GetMouseDelta();

    if (mouseDelta.x == 0.0f && mouseDelta.y == 0.0f) return;

    mouseDelta *= mouseSensitivity;

    yaw   += mouseDelta.x;
    pitch -= mouseDelta.y; // Inverted Y-axis

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(direction);
}

void Camera::ProcessGamepadInput(InputSystem* input, float deltaTime) {
    if (!input->IsGamepadConnected(0)) return;

    const float cameraSpeed = movementSpeed * deltaTime;

    // Left stick for movement
    glm::vec2 leftStick = input->GetGamepadLeftStick(0);
    if (glm::length(leftStick) > 0.0f) {
        glm::vec3 movement(0.0f);
        movement += front * leftStick.y; // Forward/Backward
        movement += glm::normalize(glm::cross(front, up)) * leftStick.x; // Left/Right
        position += movement * cameraSpeed;
    }

    // Right stick for camera rotation
    glm::vec2 rightStick = input->GetGamepadRightStick(0);
    if (glm::length(rightStick) > 0.0f) {
        float gamepadSensitivity = 100.0f; // Gamepad needs higher sensitivity
        yaw   += rightStick.x * gamepadSensitivity * deltaTime;
        pitch -= rightStick.y * gamepadSensitivity * deltaTime;

        if(pitch > 89.0f) pitch = 89.0f;
        if(pitch < -89.0f) pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(direction);
    }

    // Triggers for up/down movement
    float leftTrigger = input->GetGamepadLeftTrigger(0);
    float rightTrigger = input->GetGamepadRightTrigger(0);
    if (leftTrigger > 0.1f)
        position -= up * cameraSpeed * leftTrigger;
    if (rightTrigger > 0.1f)
        position += up * cameraSpeed * rightTrigger;
}

// ==================== Legacy Methods (Deprecated) ====================

void Camera::ProcessKeyboard(int key, float deltaTime) {
    glm::vec3 movement(0.0f);
    const float cameraSpeed = movementSpeed * deltaTime;

    if (key == 'W')
        movement += front;
    if (key == 'S')
        movement -= front;
    if (key == 'A')
        movement -= glm::normalize(glm::cross(front, up));
    if (key == 'D')
        movement += glm::normalize(glm::cross(front, up));
    if (key == 'Q')
        movement += up;
    if (key == 'E')
        movement -= up;
    if (glm::length(movement) > 0.0f)
        position += glm::normalize(movement) * cameraSpeed;

    UpdateViewMatrix();
}

void Camera::ProcessMouseMovement(double xoffset, double yoffset) {
    float sensitivity = mouseSensitivity;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += static_cast<float>(xoffset);
    pitch += static_cast<float>(yoffset);

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(direction);

    UpdateViewMatrix();
}
