#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class InputSystem; // Forward declaration

class Camera {
private:
    glm::vec3 cameraTarget = {0.0f, 0.0f, 0.0f};
    glm::vec3 cameraDirection;
    glm::vec3 cameraRight;
    glm::vec3 cameraUp;

    bool m_InputEnabled = true;

public:
    float yaw, pitch;
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::mat4 view;

    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    float movementSpeed = 2.5f;
    float mouseSensitivity = 0.1f;

    Camera();
    void UpdateViewMatrix();
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspect) const;
    float GetNear() const { return nearPlane; }
    float GetFar()  const { return farPlane;  }

    // New Input System methods
    void Update(InputSystem* input, float deltaTime);
    void ProcessKeyboardInput(InputSystem* input, float deltaTime);
    void ProcessMouseInput(InputSystem* input);
    void ProcessGamepadInput(InputSystem* input, float deltaTime);

    // Legacy methods for backward compatibility (deprecated)
    void ProcessKeyboard(int key, float deltaTime);
    void ProcessMouseMovement(double xoffset, double yoffset);

    // Input control
    void SetInputEnabled(bool enabled) { m_InputEnabled = enabled; }
    bool IsInputEnabled() const { return m_InputEnabled; }
};