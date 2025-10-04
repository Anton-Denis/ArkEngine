//
// Created by Anton on 10.01.2025.
//
#include "../../include/core/InputSystem.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include <iostream>
#include <cmath>

// Static instance for callbacks
InputSystem* InputSystem::s_Instance = nullptr;

InputSystem::InputSystem(GLFWwindow* window)
    : m_Window(window)
    , m_FirstMouse(true)
    , m_InputEnabled(true)
    , m_CameraInputEnabled(true)
    , m_CursorVisible(true)
{
    s_Instance = this;

    // Initialize mouse state
    m_MouseState = {};
    m_MouseStateLastFrame = {};
    m_LastMousePosition = glm::vec2(0.0f);

    // Initialize gamepad states
    for (int i = 0; i < MAX_GAMEPADS; ++i) {
        m_GamepadStates[i] = {};
        m_GamepadStates[i].connected = false;
        m_GamepadStates[i].deadzone = 0.15f; // 15% deadzone by default
        for (int j = 0; j < 15; ++j) {
            m_GamepadStates[i].buttons[j] = false;
            m_GamepadStates[i].buttonsLastFrame[j] = false;
        }
        for (int j = 0; j < 6; ++j) {
            m_GamepadStates[i].axes[j] = 0.0f;
        }
    }

    // Set up GLFW callbacks
    glfwSetKeyCallback(m_Window, KeyCallback);
    glfwSetMouseButtonCallback(m_Window, MouseButtonCallback);
    glfwSetCursorPosCallback(m_Window, CursorPosCallback);
    glfwSetScrollCallback(m_Window, ScrollCallback);
    glfwSetJoystickCallback(GamepadCallback);

    // Check for connected gamepads
    for (int i = 0; i < MAX_GAMEPADS; ++i) {
        if (glfwJoystickPresent(GLFW_JOYSTICK_1 + i) && glfwJoystickIsGamepad(GLFW_JOYSTICK_1 + i)) {
            m_GamepadStates[i].connected = true;
            m_GamepadStates[i].name = glfwGetGamepadName(GLFW_JOYSTICK_1 + i);
            std::cout << "Gamepad " << i << " connected: " << m_GamepadStates[i].name << std::endl;
        }
    }

    // Initialize cursor to normal mode
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

InputSystem::~InputSystem() {
    s_Instance = nullptr;
}

void InputSystem::Update() {
    if (!m_InputEnabled) return;

    // Store last frame states
    m_KeyStatesLastFrame = m_KeyStates;
    m_MouseStateLastFrame = m_MouseState;

    // Update all input states
    UpdateKeyboardState();
    UpdateMouseState();
    UpdateGamepadState();
    UpdateTouchState();
}

void InputSystem::LateUpdate() {
    // Reset per-frame data
    m_MouseState.delta = glm::vec2(0.0f);
    m_MouseState.scroll = glm::vec2(0.0f);
}

// ==================== Keyboard Input ====================

void InputSystem::UpdateKeyboardState() {
    // GLFW callbacks handle key state changes
}

bool InputSystem::IsKeyPressed(int key) const {
    if (!m_InputEnabled) return false;
    auto it = m_KeyStates.find(key);
    return it != m_KeyStates.end() && it->second;
}

bool InputSystem::IsKeyJustPressed(int key) const {
    if (!m_InputEnabled) return false;
    auto current = m_KeyStates.find(key);
    auto last = m_KeyStatesLastFrame.find(key);
    bool currentPressed = (current != m_KeyStates.end() && current->second);
    bool lastPressed = (last != m_KeyStatesLastFrame.end() && last->second);
    return currentPressed && !lastPressed;
}

bool InputSystem::IsKeyJustReleased(int key) const {
    if (!m_InputEnabled) return false;
    auto current = m_KeyStates.find(key);
    auto last = m_KeyStatesLastFrame.find(key);
    bool currentPressed = (current != m_KeyStates.end() && current->second);
    bool lastPressed = (last != m_KeyStatesLastFrame.end() && last->second);
    return !currentPressed && lastPressed;
}

// ==================== Mouse Input ====================

void InputSystem::UpdateMouseState() {
    double xpos, ypos;
    glfwGetCursorPos(m_Window, &xpos, &ypos);

    glm::vec2 currentPos(static_cast<float>(xpos), static_cast<float>(ypos));

    if (m_FirstMouse) {
        m_LastMousePosition = currentPos;
        m_FirstMouse = false;
    }

    m_MouseState.position = currentPos;
    m_MouseState.delta = currentPos - m_LastMousePosition;
    m_LastMousePosition = currentPos;

    // Update button states via callbacks
}

bool InputSystem::IsMouseButtonPressed(int button) const {
    if (!m_InputEnabled) return false;
    if (button < 0 || button >= 8) return false;
    return m_MouseState.buttons[button];
}

bool InputSystem::IsMouseButtonJustPressed(int button) const {
    if (!m_InputEnabled) return false;
    if (button < 0 || button >= 8) return false;
    return m_MouseState.buttons[button] && !m_MouseStateLastFrame.buttons[button];
}

bool InputSystem::IsMouseButtonJustReleased(int button) const {
    if (!m_InputEnabled) return false;
    if (button < 0 || button >= 8) return false;
    return !m_MouseState.buttons[button] && m_MouseStateLastFrame.buttons[button];
}

glm::vec2 InputSystem::GetMousePosition() const {
    return m_MouseState.position;
}

glm::vec2 InputSystem::GetMouseDelta() const {
    if (!m_InputEnabled || !m_CameraInputEnabled) return glm::vec2(0.0f);
    return m_MouseState.delta;
}

glm::vec2 InputSystem::GetRawMouseDelta() const {
    if (!m_InputEnabled) return glm::vec2(0.0f);
    return m_MouseState.delta; // unabhängig vom Kamera-Flag
}

glm::vec2 InputSystem::GetMouseScroll() const {
    if (!m_InputEnabled) return glm::vec2(0.0f);
    return m_MouseState.scroll;
}

void InputSystem::SetCursorVisible(bool visible) {
    m_CursorVisible = visible;
    if (visible) {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
}

bool InputSystem::IsCursorVisible() const {
    return m_CursorVisible;
}

void InputSystem::SetCursorMode(bool locked) {
    if (locked) {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_CursorVisible = false;
        m_FirstMouse = true; // Reset to avoid camera jump
    } else {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_CursorVisible = true;
    }
}

// ==================== Gamepad Input ====================

void InputSystem::UpdateGamepadState() {
    for (int i = 0; i < MAX_GAMEPADS; ++i) {
        if (!glfwJoystickPresent(GLFW_JOYSTICK_1 + i)) {
            if (m_GamepadStates[i].connected) {
                m_GamepadStates[i].connected = false;
                std::cout << "Gamepad " << i << " disconnected" << std::endl;
            }
            continue;
        }

        if (!glfwJoystickIsGamepad(GLFW_JOYSTICK_1 + i)) {
            continue;
        }

        if (!m_GamepadStates[i].connected) {
            m_GamepadStates[i].connected = true;
            m_GamepadStates[i].name = glfwGetGamepadName(GLFW_JOYSTICK_1 + i);
            std::cout << "Gamepad " << i << " connected: " << m_GamepadStates[i].name << std::endl;
        }

        GLFWgamepadstate state;
        if (glfwGetGamepadState(GLFW_JOYSTICK_1 + i, &state)) {
            // Store last frame button states
            for (int j = 0; j < 15; ++j) {
                m_GamepadStates[i].buttonsLastFrame[j] = m_GamepadStates[i].buttons[j];
                m_GamepadStates[i].buttons[j] = state.buttons[j] == GLFW_PRESS;
            }

            // Store axis states with deadzone
            for (int j = 0; j < 6; ++j) {
                m_GamepadStates[i].axes[j] = ApplyDeadzone(state.axes[j], m_GamepadStates[i].deadzone);
            }
        }
    }
}

bool InputSystem::IsGamepadConnected(int gamepadID) const {
    if (gamepadID < 0 || gamepadID >= MAX_GAMEPADS) return false;
    return m_GamepadStates[gamepadID].connected;
}

bool InputSystem::IsGamepadButtonPressed(Input::GamepadButton button, int gamepadID) const {
    if (!m_InputEnabled || !IsGamepadConnected(gamepadID)) return false;
    int buttonIndex = static_cast<int>(button);
    if (buttonIndex < 0 || buttonIndex >= 15) return false;
    return m_GamepadStates[gamepadID].buttons[buttonIndex];
}

bool InputSystem::IsGamepadButtonJustPressed(Input::GamepadButton button, int gamepadID) const {
    if (!m_InputEnabled || !IsGamepadConnected(gamepadID)) return false;
    int buttonIndex = static_cast<int>(button);
    if (buttonIndex < 0 || buttonIndex >= 15) return false;
    return m_GamepadStates[gamepadID].buttons[buttonIndex] &&
           !m_GamepadStates[gamepadID].buttonsLastFrame[buttonIndex];
}

bool InputSystem::IsGamepadButtonJustReleased(Input::GamepadButton button, int gamepadID) const {
    if (!m_InputEnabled || !IsGamepadConnected(gamepadID)) return false;
    int buttonIndex = static_cast<int>(button);
    if (buttonIndex < 0 || buttonIndex >= 15) return false;
    return !m_GamepadStates[gamepadID].buttons[buttonIndex] &&
           m_GamepadStates[gamepadID].buttonsLastFrame[buttonIndex];
}

float InputSystem::GetGamepadAxis(Input::GamepadAxis axis, int gamepadID) const {
    if (!m_InputEnabled || !IsGamepadConnected(gamepadID)) return 0.0f;
    int axisIndex = static_cast<int>(axis);
    if (axisIndex < 0 || axisIndex >= 6) return 0.0f;
    return m_GamepadStates[gamepadID].axes[axisIndex];
}

glm::vec2 InputSystem::GetGamepadLeftStick(int gamepadID) const {
    if (!IsGamepadConnected(gamepadID)) return glm::vec2(0.0f);
    return glm::vec2(
        m_GamepadStates[gamepadID].axes[static_cast<int>(Input::GamepadAxis::LeftX)],
        m_GamepadStates[gamepadID].axes[static_cast<int>(Input::GamepadAxis::LeftY)]
    );
}

glm::vec2 InputSystem::GetGamepadRightStick(int gamepadID) const {
    if (!IsGamepadConnected(gamepadID)) return glm::vec2(0.0f);
    return glm::vec2(
        m_GamepadStates[gamepadID].axes[static_cast<int>(Input::GamepadAxis::RightX)],
        m_GamepadStates[gamepadID].axes[static_cast<int>(Input::GamepadAxis::RightY)]
    );
}

float InputSystem::GetGamepadLeftTrigger(int gamepadID) const {
    if (!IsGamepadConnected(gamepadID)) return 0.0f;
    // Convert from [-1, 1] to [0, 1]
    return (m_GamepadStates[gamepadID].axes[static_cast<int>(Input::GamepadAxis::LeftTrigger)] + 1.0f) * 0.5f;
}

float InputSystem::GetGamepadRightTrigger(int gamepadID) const {
    if (!IsGamepadConnected(gamepadID)) return 0.0f;
    // Convert from [-1, 1] to [0, 1]
    return (m_GamepadStates[gamepadID].axes[static_cast<int>(Input::GamepadAxis::RightTrigger)] + 1.0f) * 0.5f;
}

void InputSystem::SetGamepadDeadzone(float deadzone, int gamepadID) {
    if (gamepadID < 0 || gamepadID >= MAX_GAMEPADS) return;
    m_GamepadStates[gamepadID].deadzone = std::clamp(deadzone, 0.0f, 1.0f);
}

std::string InputSystem::GetGamepadName(int gamepadID) const {
    if (!IsGamepadConnected(gamepadID)) return "";
    return m_GamepadStates[gamepadID].name;
}

// ==================== Touch Input ====================

void InputSystem::UpdateTouchState() {
    // Simulate touch with mouse for desktop
    // On mobile platforms, this would use native touch APIs
    m_TouchPoints.clear();

    if (m_MouseState.buttons[GLFW_MOUSE_BUTTON_LEFT]) {
        Input::TouchPoint touch;
        touch.id = 0;
        touch.position = m_MouseState.position;
        touch.delta = m_MouseState.delta;
        touch.active = true;
        m_TouchPoints.push_back(touch);
    }
}

int InputSystem::GetTouchCount() const {
    if (!m_InputEnabled) return 0;
    return static_cast<int>(m_TouchPoints.size());
}

Input::TouchPoint InputSystem::GetTouch(int index) const {
    if (!m_InputEnabled || index < 0 || index >= static_cast<int>(m_TouchPoints.size())) {
        return Input::TouchPoint{-1, glm::vec2(0.0f), glm::vec2(0.0f), false};
    }
    return m_TouchPoints[index];
}

bool InputSystem::IsTouchActive(int touchID) const {
    if (!m_InputEnabled) return false;
    for (const auto& touch : m_TouchPoints) {
        if (touch.id == touchID && touch.active) return true;
    }
    return false;
}

// ==================== Input Control ====================

void InputSystem::SetInputEnabled(bool enabled) {
    m_InputEnabled = enabled;
}

bool InputSystem::IsInputEnabled() const {
    return m_InputEnabled;
}

void InputSystem::SetCameraInputEnabled(bool enabled) {
    m_CameraInputEnabled = enabled;
}

bool InputSystem::IsCameraInputEnabled() const {
    return m_CameraInputEnabled;
}

// ==================== Helper Functions ====================

float InputSystem::ApplyDeadzone(float value, float deadzone) const {
    if (std::abs(value) < deadzone) return 0.0f;
    // Rescale to smooth transition
    float sign = (value > 0.0f) ? 1.0f : -1.0f;
    return sign * ((std::abs(value) - deadzone) / (1.0f - deadzone));
}

// ==================== GLFW Callbacks ====================

void InputSystem::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;  // Unused parameter

    if (!s_Instance) return;

    // ✅ FIX: Forward to ImGui FIRST
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    // Then handle in InputSystem
    if (action == GLFW_PRESS) {
        s_Instance->m_KeyStates[key] = true;
        std::cout << "[InputSystem] Key pressed: " << key << std::endl;
    } else if (action == GLFW_RELEASE) {
        s_Instance->m_KeyStates[key] = false;
        std::cout << "[InputSystem] Key released: " << key << std::endl;
    }
}

void InputSystem::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;  // Unused parameter

    if (!s_Instance) return;

    // ✅ FIX: Forward to ImGui FIRST
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    // Then handle in InputSystem
    if (button >= 0 && button < 8) {
        s_Instance->m_MouseState.buttons[button] = (action == GLFW_PRESS);
    }
}

void InputSystem::CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!s_Instance) return;

    // ✅ FIX: Forward to ImGui FIRST
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    // InputSystem handles position in UpdateMouseState()
}

void InputSystem::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (!s_Instance) return;

    // ✅ FIX: Forward to ImGui FIRST
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

    // Then handle in InputSystem
    s_Instance->m_MouseState.scroll = glm::vec2(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void InputSystem::GamepadCallback(int jid, int event) {
    if (!s_Instance) return;

    int gamepadID = jid - GLFW_JOYSTICK_1;
    if (gamepadID < 0 || gamepadID >= MAX_GAMEPADS) return;

    if (event == GLFW_CONNECTED) {
        if (glfwJoystickIsGamepad(jid)) {
            s_Instance->m_GamepadStates[gamepadID].connected = true;
            s_Instance->m_GamepadStates[gamepadID].name = glfwGetGamepadName(jid);
            std::cout << "Gamepad " << gamepadID << " connected: "
                     << s_Instance->m_GamepadStates[gamepadID].name << std::endl;
        }
    } else if (event == GLFW_DISCONNECTED) {
        s_Instance->m_GamepadStates[gamepadID].connected = false;
        std::cout << "Gamepad " << gamepadID << " disconnected" << std::endl;
    }
}
