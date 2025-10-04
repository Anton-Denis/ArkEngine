//
// Created by Anton on 10.08.2025.
//
#include "glad/glad.h"
#include "../../include/objects/Grid.hpp"
#include "glm/gtc/matrix_transform.hpp"

Grid::Grid() : GameObject() {
    Initialize();
}

Grid::~Grid() {
    Cleanup();
}

void Grid::Initialize() {
    if (initialized) return;

    LoadShaders();
    CreateFullscreenTriangle();
    initialized = true;
}

void Grid::LoadShaders() {
    gridShader = ResourceManager::GetShader("shaders/WorldGrid.vert", "shaders/WorldGrid.frag");
}

void Grid::CreateFullscreenTriangle() {
    // Großes Dreieck für Fullscreen (effizienter als Quad)
    float vertices[] = {
            -1.0f, -1.0f,
            3.0f, -1.0f,
            -1.0f,  3.0f
    };

    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Grid::Render(const Camera& camera, float aspect) {
    if (!initialized) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);

    gridShader->Use();

    // Inverse View-Projection Matrix berechnen
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = camera.GetProjectionMatrix(aspect);
    glm::mat4 invViewProj = glm::inverse(projection * view);

    // Uniforms setzen
    gridShader->SetMat4("uInvViewProj", invViewProj);
    gridShader->SetVec3("uCameraPos", camera.position);
    gridShader->SetFloat("uPlaneY", planeY);
    gridShader->SetFloat("uMajorStep", majorStep);
    gridShader->SetFloat("uThicknessPx", thicknessPx);
    gridShader->SetFloat("uFadeStart", fadeStart);
    gridShader->SetFloat("uFadeEnd", fadeEnd);

    // Grid-Farben
    gridShader->SetVec3("uGridColorMinor", glm::vec3(0.55f));
    gridShader->SetVec3("uGridColorMajor", glm::vec3(0.85f));
    gridShader->SetVec3("uBackground", glm::vec3(0.0f));
    gridShader->SetVec3("uAxisX", glm::vec3(0.2f, 0.2f, 0.2f));
    gridShader->SetVec3("uAxisZ", glm::vec3(0.2f, 0.2f, 0.2f));
    gridShader->SetVec3("uAxisOrigin", glm::vec3(0.2f));

    glBindVertexArray(gridVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
}

void Grid::Cleanup() {
    if (gridVAO) {
        glDeleteVertexArrays(1, &gridVAO);
        gridVAO = 0;
    }
    if (gridVBO) {
        glDeleteBuffers(1, &gridVBO);
        gridVBO = 0;
    }
    initialized = false;
}