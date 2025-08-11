//
// Created by Anton on 10.08.2025.
//
#pragma once
#include "../objects/GameObject.hpp"
#include "../core/Shader.hpp"
#include "../core/Camera.hpp"
#include "../core/ResourceManager.hpp"
#include <memory>

class Grid : public GameObject {
public:
    Grid();
    ~Grid();

    void Initialize();
    void Render(const Camera& camera, float aspect);
    void Cleanup();

    // Grid-Parameter
    void SetPlaneY(float y) { planeY = y; }
    void SetMajorStep(float step) { majorStep = step; }
    void SetThickness(float thickness) { thicknessPx = thickness; }
    void SetFadeDistance(float start, float end) { fadeStart = start; fadeEnd = end; }

private:
    std::shared_ptr<Shader> gridShader;
    unsigned int gridVAO, gridVBO;
    bool initialized = false;

    // Grid-Parameter
    float planeY = 0.0f;
    float majorStep = 10.0f;
    float thicknessPx = 1.0f;
    float fadeStart = 50.0f;
    float fadeEnd = 200.0f;

    void CreateFullscreenTriangle();
    void LoadShaders();
};
