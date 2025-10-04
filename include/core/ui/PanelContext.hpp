#pragma once
#include <vector>
class Scene;
struct Mesh; // forward

struct SelectionState { int selectedObject = 0; };

struct PanelContext {
    Scene* scene = nullptr;
    const std::vector<Mesh*>* meshes = nullptr; // optional
    SelectionState* selection = nullptr;        // gemeinsame Objektselektion
};
