//
// Created by Anton on 03.07.2025.
//

#ifndef INC_3DRENDERER_RENDERER_HPP
#define INC_3DRENDERER_RENDERER_HPP

#include "Window.hpp"
#include "Shader.hpp"
#include "Scene.hpp"
#include "../objects/Cube.hpp"
#include "../objects/Light.hpp"
#include "../objects/PointLight.hpp"
#include "../objects/DirectionalLight.hpp"
#include "../objects/Plane.hpp"
#include "../objects/Model.hpp"
#include "Camera.hpp"
#include "UI.hpp"
#include "../objects/Grid.hpp"

class Renderer {
public:
    Renderer(Window& window, Scene& scene, std::shared_ptr<Shader> shader, Camera& cam, UI& ui);
    void Render();
    void InitializeGrid();
private:
    double lastTime = 0.0;
    int nbFrames = 0;
    double deltaTime = 0.0f;
    double lastFrameTime = 0.0f;

    bool paused = false;
    bool escPressedLastFrame = false;

    std::vector<Mesh*> cachedMeshes;
    bool meshesDirty = true;

    GLuint viewportFBO = 0;
    GLuint viewportTexture = 0;
    GLuint viewportRBO = 0;
    int viewportWidth = 1280, viewportHeight = 720;

    unsigned int lightVAO = 0, lightVBO = 0;

    void CreateViewportFBO(int width, int height);
    void DeleteViewportFBO();
    void UpdateFPS();
    void LimitFPS(double frameStart, double targetFPS);
    void UpdateMeshCache();
    void RenderMeshes();
    void RenderGrid(float aspect);
    void SetProjectionMatrix(const glm::mat4& projection, const glm::mat4& view);
    void SetMaterials();
    void SetLighting(Shader& shader);

    void Input();

    Window& window;
    Scene& scene;
    std::shared_ptr<Shader> shader;
    Camera& camera;
    UI& ui;
    std::unique_ptr<Grid> grid;
};

#endif //INC_3DRENDERER_RENDERER_HPP
