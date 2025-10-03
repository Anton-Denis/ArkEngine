//
// Created by Anton on 04.07.2025.
//
#include "../../include/core/Camera.hpp"

Camera::Camera(GLFWwindow* window, bool hideCursor) {
    if (hideCursor)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, Camera::mouse_callback_dispatch);

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

void Camera::Movement(GLFWwindow* window, float deltaTime) {
    glm::vec3 movement(0.0f);
    const float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        movement += front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        movement -= front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        movement -= glm::normalize(glm::cross(front, up));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        movement += glm::normalize(glm::cross(front, up));
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        movement += up;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        movement -= up;
    if (glm::length(movement) > 0.0f)
        position += glm::normalize(movement) * cameraSpeed;

    UpdateViewMatrix();
}

void Camera::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (paused && *paused) return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

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

void Camera::mouse_callback_dispatch(GLFWwindow* window, double xpos, double ypos) {
    auto* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (camera)
        camera->mouse_callback(window, xpos, ypos);
}
