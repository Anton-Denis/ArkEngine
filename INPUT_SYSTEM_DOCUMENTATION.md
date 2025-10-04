# Input System Documentation

## Overview
Das **InputSystem** ist ein zentralisiertes, plattformübergreifendes Input-Management-System für die ArkEngine. Es bietet eine einheitliche Schnittstelle für Keyboard, Mouse, Gamepad und Touch-Input.

## Features

### ✅ Implementierte Features
- **Keyboard Input**: Key-Press, Key-Hold, Key-Release Detection
- **Mouse Input**: Position, Delta, Scroll, Button States
- **Gamepad Support**: Bis zu 4 Gamepads gleichzeitig
  - Buttons (A, B, X, Y, Bumpers, D-Pad, etc.)
  - Analog Sticks (Left/Right mit Deadzone)
  - Triggers (Left/Right)
  - Hot-Plug Support (Connect/Disconnect während Laufzeit)
- **Touch Input**: Simuliert via Mouse (für Mobile erweiterbar)
- **Cursor Control**: Show/Hide/Lock Cursor
- **Input Enable/Disable**: Global und für spezifische Systeme (z.B. Camera)

## Architecture

### Class Structure
```
InputSystem
├── Keyboard State Management
├── Mouse State Management  
├── Gamepad State Management (4 Gamepads)
├── Touch State Management
└── GLFW Callback Handlers
```

### Key Components

**Header**: `include/core/InputSystem.hpp`
**Implementation**: `src/core/InputSystem.cpp`

## Usage Examples

### 1. Initialization
```cpp
Window window(1920, 1080, "Game");
InputSystem inputSystem(window.GetWindow());
```

### 2. Keyboard Input
```cpp
// In your update loop
inputSystem.Update();

// Check if key is currently pressed
if (inputSystem.IsKeyPressed(GLFW_KEY_W)) {
    // Move forward
}

// Check if key was just pressed this frame
if (inputSystem.IsKeyJustPressed(GLFW_KEY_SPACE)) {
    // Jump
}

// Check if key was just released this frame
if (inputSystem.IsKeyJustReleased(GLFW_KEY_E)) {
    // Stop interaction
}
```

### 3. Mouse Input
```cpp
// Get mouse position
glm::vec2 mousePos = inputSystem.GetMousePosition();

// Get mouse movement delta (useful for camera)
glm::vec2 mouseDelta = inputSystem.GetMouseDelta();

// Get scroll wheel
glm::vec2 scroll = inputSystem.GetMouseScroll();

// Check mouse buttons
if (inputSystem.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
    // Handle left click
}
```

### 4. Cursor Control
```cpp
// Lock cursor for FPS camera (hides and captures cursor)
inputSystem.SetCursorMode(true);

// Unlock cursor for UI interaction
inputSystem.SetCursorMode(false);

// Just hide cursor without locking
inputSystem.SetCursorVisible(false);
```

### 5. Gamepad Input
```cpp
// Check if gamepad is connected
if (inputSystem.IsGamepadConnected(0)) {
    
    // Get left stick for movement
    glm::vec2 leftStick = inputSystem.GetGamepadLeftStick(0);
    position += leftStick * speed;
    
    // Get right stick for camera rotation
    glm::vec2 rightStick = inputSystem.GetGamepadRightStick(0);
    yaw += rightStick.x;
    pitch += rightStick.y;
    
    // Check button presses
    if (inputSystem.IsGamepadButtonJustPressed(Input::GamepadButton::A, 0)) {
        Jump();
    }
    
    // Get trigger values (0.0 to 1.0)
    float leftTrigger = inputSystem.GetGamepadLeftTrigger(0);
    float rightTrigger = inputSystem.GetGamepadRightTrigger(0);
    
    // Set custom deadzone (default: 0.15 = 15%)
    inputSystem.SetGamepadDeadzone(0.2f, 0);
}
```

### 6. Camera Integration
```cpp
// In Renderer.cpp
inputSystem->Update();

// Toggle pause mode
if (inputSystem->IsKeyJustPressed(GLFW_KEY_ESCAPE)) {
    paused = !paused;
    inputSystem->SetCursorMode(!paused);  // Lock cursor when playing
    camera.SetInputEnabled(!paused);
    inputSystem->SetCameraInputEnabled(!paused);
}

// Update camera
if (!paused) {
    camera.Update(inputSystem, deltaTime);
}

// Reset per-frame data
inputSystem->LateUpdate();
```

### 7. Touch Input (Desktop Simulation)
```cpp
// Get touch count
int touchCount = inputSystem.GetTouchCount();

// Get first touch
if (touchCount > 0) {
    Input::TouchPoint touch = inputSystem.GetTouch(0);
    glm::vec2 touchPos = touch.position;
    glm::vec2 touchDelta = touch.delta;
}

// Check if specific touch is active
if (inputSystem.IsTouchActive(0)) {
    // Handle touch
}
```

## Integration in Existing Classes

### Camera Class
Die Camera-Klasse wurde refaktoriert, um das InputSystem zu verwenden:

```cpp
// Old method (deprecated)
void ProcessKeyboard(int key, float deltaTime);
void ProcessMouseMovement(double xoffset, double yoffset);

// New method
void Update(InputSystem* input, float deltaTime);
void ProcessKeyboardInput(InputSystem* input, float deltaTime);
void ProcessMouseInput(InputSystem* input);
void ProcessGamepadInput(InputSystem* input, float deltaTime);

// Input control
void SetInputEnabled(bool enabled);
bool IsInputEnabled() const;
```

### Renderer Class
Der Renderer verwendet jetzt das InputSystem:

```cpp
Renderer(Window& window, Scene& scene, std::shared_ptr<Shader> shader, 
         Camera& cam, UI& ui, InputSystem* inputSys);
```

## Control Flow

```
Frame Start
    ↓
inputSystem->Update()
    ├── UpdateKeyboardState()
    ├── UpdateMouseState()
    ├── UpdateGamepadState()
    └── UpdateTouchState()
    ↓
Game Logic / Input Handling
    ├── Check Keys/Buttons
    ├── Get Axis Values
    └── Update Game State
    ↓
Rendering
    ↓
inputSystem->LateUpdate()  // Reset per-frame data
    ↓
Frame End
```

## Gamepad Button Mapping

```cpp
enum class GamepadButton {
    A, B, X, Y,
    LeftBumper, RightBumper,
    Back, Start, Guide,
    LeftThumb, RightThumb,
    DPadUp, DPadRight, DPadDown, DPadLeft
};
```

## Gamepad Axis Mapping

```cpp
enum class GamepadAxis {
    LeftX, LeftY,        // Left analog stick
    RightX, RightY,      // Right analog stick
    LeftTrigger,         // Left trigger (0.0 - 1.0)
    RightTrigger         // Right trigger (0.0 - 1.0)
};
```

## Best Practices

### 1. Call Update() Once Per Frame
```cpp
// In main game loop
inputSystem.Update();
// ... game logic ...
inputSystem.LateUpdate();
```

### 2. Use IsKeyJustPressed for Toggle Actions
```cpp
// Good: Toggle only once per key press
if (inputSystem.IsKeyJustPressed(GLFW_KEY_P)) {
    paused = !paused;
}

// Bad: Will toggle every frame while held
if (inputSystem.IsKeyPressed(GLFW_KEY_P)) {
    paused = !paused;
}
```

### 3. Check Gamepad Connection Before Use
```cpp
if (inputSystem.IsGamepadConnected(0)) {
    // Safe to use gamepad
}
```

### 4. Adjust Gamepad Deadzone for Better Feel
```cpp
// Increase deadzone for older controllers
inputSystem.SetGamepadDeadzone(0.25f, 0);
```

## Advanced Features

### Multiple Gamepads
Das System unterstützt bis zu 4 Gamepads gleichzeitig:

```cpp
for (int i = 0; i < 4; i++) {
    if (inputSystem.IsGamepadConnected(i)) {
        // Handle player i input
        glm::vec2 movement = inputSystem.GetGamepadLeftStick(i);
    }
}
```

### Hot-Plug Support
Gamepads können während der Laufzeit an-/abgesteckt werden:
- Callback wird automatisch ausgelöst
- Console-Output zeigt Connection-Status
- Keine zusätzliche Logik erforderlich

## Platform Support

- **Windows**: ✅ Vollständig unterstützt
- **Linux**: ✅ Vollständig unterstützt
- **macOS**: ✅ Vollständig unterstützt
- **Mobile**: 🔄 Touch-Simulation vorhanden, native Touch-API erweiterbar

## Performance

- **Memory**: ~4KB für 4 Gamepads + Input States
- **CPU**: Minimal (nur State-Tracking, keine Polling)
- **Callbacks**: Event-basiert über GLFW

## Future Extensions

### Mögliche Erweiterungen:
1. **Action Mapping System**: Abstrakte Actions statt direkter Keys
2. **Input Recording/Playback**: Für Replays
3. **Vibration Support**: Gamepad Rumble
4. **Native Touch API**: iOS/Android Multi-Touch
5. **Input Rebinding**: Runtime Key-Remapping
6. **Input Profiles**: Speichern/Laden von Konfigurationen

## Troubleshooting

### Gamepad wird nicht erkannt
```cpp
// Check GLFW gamepad database
// Update gamepad mappings if needed
glfwUpdateGamepadMappings(mapping_string);
```

### Maus-Input funktioniert nicht
```cpp
// Stelle sicher, dass Update() und LateUpdate() aufgerufen werden
inputSystem.Update();
// ... game logic ...
inputSystem.LateUpdate();  // Important!
```

### Kamera springt beim Cursor-Lock
```cpp
// Das System setzt automatisch m_FirstMouse = true
// beim Cursor-Lock, um Sprünge zu vermeiden
inputSystem.SetCursorMode(true);
```

## Migration Guide

### Von altem Input-Code zu InputSystem

**Alt:**
```cpp
// In Camera
if (key == 'W') position += front * speed;

// In Renderer
if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    paused = true;
```

**Neu:**
```cpp
// In Camera
if (input->IsKeyPressed(GLFW_KEY_W))
    position += front * speed;

// In Renderer
if (inputSystem->IsKeyJustPressed(GLFW_KEY_ESCAPE))
    paused = !paused;
```

## Contact & Support

Bei Fragen oder Problemen siehe:
- Source Code: `include/core/InputSystem.hpp`
- Examples: `src/core/Camera.cpp`, `src/core/Renderer.cpp`

