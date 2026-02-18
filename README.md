# 3DRenderer

A compact C++20/OpenGL game engine prototype with an immediate-mode editor UI powered by Dear ImGui. The goal is to provide a clean, modular foundation for experimentation with rendering, scene management, and tooling while keeping dependencies lightweight.

![App Screenshot](screenshots/Screenshot 2026-02-18 030030.png)

## Highlights
- Rendering: Phong lighting with Point, Directional, and Spot lights
- Scene & Editor: Scene Hierarchy, Inspector, Menu Bar, Style Editor
- Asset Workflow: Asset Browser with folder navigation, file grid, context actions
- Monitoring: Real-time metrics (CPU time, frame time, FPS, CPU/GPU usage, RAM/VRAM) with graphs
- Cross-platform foundations via GLFW, GLAD, GLM, stb_image, Assimp, and ImGui

## Architecture Overview
- Core modules in `src/core` and headers in `include/core`
  - Renderer & Shader: Draw calls, material/shader management, uniform updates
  - Scene: GameObject tree, transforms, component-like lights and models
  - ResourceManager: Loading textures/meshes/shaders
  - ProjectManager: Project root, asset import helpers
  - UI panels: Modular ImGui panels under `src/core/ui` and `include/core/ui`
- External libs in `external/` (vendorized where practical)
- Shaders in `shaders/` (GLSL)
- Assets in `resources/` (fonts, icons, images, models)

## UI Panels
- MenuBarPanel: Global actions and project controls
- AssetBrowserPanel: Directory tree, file grid, drag & drop, rename/delete
- SceneHierarchyPanel: Object listing, selection, context menu (add/delete)
- InspectorPanel: Edit selected object properties (transform, material, light params)
- MonitoringPanel: Performance graphs and system metrics
- StyleEditorPanel: Theme customization for ImGui

## Rendering & Lighting
- Phong shading pipeline
- Supported lights: Point, Directional, Spot
- Uniform updates in the Renderer and Shader classes
- Extendable material system (textures, parameters)

## Build
Requirements:
- CMake 3.20+
- Modern C++ compiler (MSVC, MinGW, Clang, GCC) with C++20
- Windows (tested) or Linux/macOS with equivalent toolchain

Steps:
1. Clone the repository
2. Configure:
   - `cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release`
3. Build:
   - `cmake --build cmake-build-release --target 3DRenderer`

Debug build:
- `cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug`
- `cmake --build cmake-build-debug --target 3DRenderer`

## Run
- On Windows, run `cmake-build-release/3DRenderer.exe`
- Default project structure expects an `assets/` directory under your project root

## Directory Layout (excerpt)
- `src/` – C++ sources
- `include/` – headers
- `external/` – 3rd-party libraries (glad, imgui, stb, assimp, glfw, glm)
- `shaders/` – GLSL shader files
- `resources/` – fonts, icons, images, models

## Conventions & Best Practices
- Use RAII and smart pointers for lifetime management
- Keep UI logic inside panel classes; avoid monolithic UI code
- Prefer explicit resource ownership via ResourceManager
- Separate GPU state mutations from scene logic

## Roadmap
- PBR material pipeline and IBL
- ECS-style components and systems
- Editor: gizmos, undo/redo, prefab workflow
- Asset import pipeline improvements and metadata
- Cross-platform validation and CI

## License
Internal use. External libraries follow their respective licenses.
