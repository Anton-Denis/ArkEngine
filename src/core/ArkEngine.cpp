//
// Created by Anton on 05.07.2025.
//
#include "../../include/core/ArkEngine.hpp"
#include "../../include/core/InputSystem.hpp"

ArkEngine::ArkEngine() {}

ArkEngine::~ArkEngine() {}

void ArkEngine::Run() {

    ProjectManager& pm = ProjectManager::Instance();
    pm.CreateProject("TestProject");

    Window window (1920, 1080, "ArkEngine");

    // Initialize new InputSystem
    InputSystem inputSystem(window.GetWindow());
    Camera camera;

    auto shader = ResourceManager::GetShader("shaders/StandardLit.vert", "shaders/StandardLit.frag");

    Scene scene;

    UI ui(&window,window.GetWindow());

    Renderer renderer(window, scene, shader, camera, ui, &inputSystem);
    renderer.InitializeGrid();
    renderer.Render();
}