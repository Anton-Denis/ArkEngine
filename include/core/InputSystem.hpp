//
// Created by Anton on 10.01.2025.
//
#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include <string>

// Forward declarations
struct GLFWwindow;
struct GLFWgamepadstate;

namespace Input {

    // Input Device Types
    enum class DeviceType {
        Keyboard,
        Mouse,
        Gamepad,
        Touch
    };

    // Gamepad Button Mapping
    enum class GamepadButton {
        A = GLFW_GAMEPAD_BUTTON_A,
        B = GLFW_GAMEPAD_BUTTON_B,
        X = GLFW_GAMEPAD_BUTTON_X,
        Y = GLFW_GAMEPAD_BUTTON_Y,
        LeftBumper = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,
        RightBumper = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
        Back = GLFW_GAMEPAD_BUTTON_BACK,
        Start = GLFW_GAMEPAD_BUTTON_START,
        Guide = GLFW_GAMEPAD_BUTTON_GUIDE,
        LeftThumb = GLFW_GAMEPAD_BUTTON_LEFT_THUMB,
        RightThumb = GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,
        DPadUp = GLFW_GAMEPAD_BUTTON_DPAD_UP,
        DPadRight = GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
        DPadDown = GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
        DPadLeft = GLFW_GAMEPAD_BUTTON_DPAD_LEFT
    };

    // Gamepad Axis Mapping
    enum class GamepadAxis {
        LeftX = GLFW_GAMEPAD_AXIS_LEFT_X,
        LeftY = GLFW_GAMEPAD_AXIS_LEFT_Y,
        RightX = GLFW_GAMEPAD_AXIS_RIGHT_X,
        RightY = GLFW_GAMEPAD_AXIS_RIGHT_Y,
        LeftTrigger = GLFW_GAMEPAD_AXIS_LEFT_TRIGGER,
        RightTrigger = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER
    };

    // Touch Point Structure
    struct TouchPoint {
        int id;
        glm::vec2 position;
        glm::vec2 delta;
        bool active;
    };

    // Mouse Button State
    struct MouseState {
        glm::vec2 position;
        glm::vec2 delta;
        glm::vec2 scroll;
        bool buttons[8]; // Support for up to 8 mouse buttons
        bool visible;
    };

    // Gamepad State
    struct GamepadState {
        bool connected;
        std::string name;
        bool buttons[15];
        float axes[6];
        bool buttonsLastFrame[15];
        float deadzone;
    };
}

class InputSystem {
public:
    InputSystem(GLFWwindow* window);
    ~InputSystem();

    // Core Update
    void Update();
    void LateUpdate();

    // Keyboard Input
    bool IsKeyPressed(int key) const;
    bool IsKeyJustPressed(int key) const;
    bool IsKeyJustReleased(int key) const;

    // Mouse Input
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonJustPressed(int button) const;
    bool IsMouseButtonJustReleased(int button) const;
    glm::vec2 GetMousePosition() const;
    glm::vec2 GetMouseDelta() const;
    glm::vec2 GetRawMouseDelta() const; // NEW: ignores camera input enabled flag
    glm::vec2 GetMouseScroll() const;

    // Cursor Control
    void SetCursorVisible(bool visible);
    bool IsCursorVisible() const;
    void SetCursorMode(bool locked); // true = locked/hidden, false = normal

    // Gamepad Input
    bool IsGamepadConnected(int gamepadID = 0) const;
    bool IsGamepadButtonPressed(Input::GamepadButton button, int gamepadID = 0) const;
    bool IsGamepadButtonJustPressed(Input::GamepadButton button, int gamepadID = 0) const;
    bool IsGamepadButtonJustReleased(Input::GamepadButton button, int gamepadID = 0) const;
    float GetGamepadAxis(Input::GamepadAxis axis, int gamepadID = 0) const;
    glm::vec2 GetGamepadLeftStick(int gamepadID = 0) const;
    glm::vec2 GetGamepadRightStick(int gamepadID = 0) const;
    float GetGamepadLeftTrigger(int gamepadID = 0) const;
    float GetGamepadRightTrigger(int gamepadID = 0) const;
    void SetGamepadDeadzone(float deadzone, int gamepadID = 0);
    std::string GetGamepadName(int gamepadID = 0) const;

    // Touch Input (Simulated via Mouse for Desktop)
    int GetTouchCount() const;
    Input::TouchPoint GetTouch(int index) const;
    bool IsTouchActive(int touchID) const;

    // Input Enable/Disable
    void SetInputEnabled(bool enabled);
    bool IsInputEnabled() const;

    // Camera Input Control
    void SetCameraInputEnabled(bool enabled);
    bool IsCameraInputEnabled() const;

private:
    GLFWwindow* m_Window;

    // Keyboard State
    std::unordered_map<int, bool> m_KeyStates;
    std::unordered_map<int, bool> m_KeyStatesLastFrame;

    // Mouse State
    Input::MouseState m_MouseState;
    Input::MouseState m_MouseStateLastFrame;
    glm::vec2 m_LastMousePosition;
    bool m_FirstMouse;

    // Gamepad States (Support up to 4 gamepads)
    static constexpr int MAX_GAMEPADS = 4;
    Input::GamepadState m_GamepadStates[MAX_GAMEPADS];

    // Touch State (Simulated)
    std::vector<Input::TouchPoint> m_TouchPoints;

    // Control Flags
    bool m_InputEnabled;
    bool m_CameraInputEnabled;
    bool m_CursorVisible;

    // GLFW Callbacks
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void GamepadCallback(int jid, int event);

    // Helper Functions
    void UpdateKeyboardState();
    void UpdateMouseState();
    void UpdateGamepadState();
    void UpdateTouchState();
    float ApplyDeadzone(float value, float deadzone) const;

    // Static instance for callbacks
    static InputSystem* s_Instance;
};
