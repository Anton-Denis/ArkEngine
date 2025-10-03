//
// Created by Anton on 03.07.2025.
//
#include "glad/glad.h"
#include "../../include/core/Renderer.hpp"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <sstream>
#include <thread>
#include <map>

Renderer::Renderer(Window& win, Scene& sc, std::shared_ptr<Shader> sh, Camera& cam, UI& ui)
        : window(win), scene(sc), shader(sh), camera(cam), ui(ui) {
    camera.paused = &paused;
    glEnable(GL_DEPTH_TEST);
}

void Renderer::Input() {
    if (glfwGetKey(window.GetWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (!escPressedLastFrame) {
            paused = !paused;
            if (paused)
                glfwSetInputMode(window.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            else {
                glfwSetInputMode(window.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                double xpos, ypos;
                glfwGetCursorPos(window.GetWindow(), &xpos, &ypos);
                camera.lastX = xpos;
                camera.lastY = ypos;
                camera.firstMouse = true;
            }
        }
        escPressedLastFrame = true;
    } else {
        escPressedLastFrame = false;
    }
    if (!paused) {
        camera.Movement(window.GetWindow(), deltaTime);
    }
}

void Renderer::UpdateFPS() {
    double currentTime = glfwGetTime();
    nbFrames++;
    if (currentTime - lastTime >= 1.0) {
        std::stringstream ss;
        ss << "3D Renderer - FPS: " << nbFrames;
        glfwSetWindowTitle(window.GetWindow(), ss.str().c_str());
        nbFrames = 0;
        lastTime += 1.0;
    }
}

void Renderer::LimitFPS(double frameStart, double targetFPS) {
    if (targetFPS <= 0.0) return;

    double targetFrameTime = 1.0 / targetFPS;
    double elapsed;

    do {
        elapsed = glfwGetTime() - frameStart;
        if (elapsed < targetFrameTime) {
            std::this_thread::yield();
        }
    } while (elapsed < targetFrameTime);
}

// Renderer.cpp
void Renderer::CreateViewportFBO(int width, int height) {
    if (viewportFBO) DeleteViewportFBO();

    glGenFramebuffers(1, &viewportFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, viewportFBO);

    glGenTextures(1, &viewportTexture);
    glBindTexture(GL_TEXTURE_2D, viewportTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, viewportTexture, 0);

    glGenRenderbuffers(1, &viewportRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, viewportRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, viewportRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer nicht komplett!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DeleteViewportFBO() {
    if (viewportTexture) glDeleteTextures(1, &viewportTexture);
    if (viewportRBO) glDeleteRenderbuffers(1, &viewportRBO);
    if (viewportFBO) glDeleteFramebuffers(1, &viewportFBO);
    viewportTexture = 0;
    viewportRBO = 0;
    viewportFBO = 0;
}

glm::mat4 ComputeModelMatrix(const GameObject& obj) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, obj.GetPosition());
    model *= glm::mat4_cast(obj.GetRotation());
    model = glm::scale(model, obj.GetScale());
    return model;
}

void Renderer::UpdateMeshCache() {
    if (!meshesDirty) return;
    cachedMeshes.clear();
    for (auto& obj : scene.GetObjects()) {
        if (auto* cube = dynamic_cast<Cube*>(obj.get())) {
            if (cube->GetMesh()) cachedMeshes.push_back(cube->GetMesh().get());
        } else if (auto* plane = dynamic_cast<Plane*>(obj.get())) {
            if (plane->GetMesh()) cachedMeshes.push_back(plane->GetMesh().get());
        }
    }
    meshesDirty = false;
}

void Renderer::SetProjectionMatrix(const glm::mat4& projection, const glm::mat4& view) {
    shader->SetMat4("projection", projection);
    shader->SetMat4("view", view);
    shader->SetVec3("viewPos", camera.position);
}

void Renderer::SetMaterials() {
    shader->SetFloat("material.shininess", 32.0f);
    shader->SetInt("material.diffuse", 0);
    shader->SetInt("material.specular", 1);
}

void Renderer::SetLighting(Shader& shader) {
    int pointLightIdx = 0;
    int numPointLights = 0;
    bool hasDirLight = false;

    std::vector<Light*> pointLights;
    Light* dirLight = nullptr;

    for (const auto& obj : scene.GetObjects()) {
        if (auto* light = dynamic_cast<Light*>(obj.get())) {
            if (light->type == Light::Type::Point && pointLightIdx < 16) {
                pointLights.push_back(light);
                ++pointLightIdx;
            } else if (light->type == Light::Type::Directional && !dirLight) {
                dirLight = light;
            }
        }
    }
    numPointLights = static_cast<int>(pointLights.size());

    for (int i = 0; i < numPointLights; ++i) {
        auto* light = pointLights[i];
        shader.SetVec3("pointLights[" + std::to_string(i) + "].position", light->GetPosition());
        shader.SetVec3("pointLights[" + std::to_string(i) + "].ambient",  light->color * 0.1f);
        shader.SetVec3("pointLights[" + std::to_string(i) + "].diffuse",  light->color * 0.8f);
        shader.SetVec3("pointLights[" + std::to_string(i) + "].specular", light->color * 1.0f);
        shader.SetFloat("pointLights[" + std::to_string(i) + "].constant",  light->constant);
        shader.SetFloat("pointLights[" + std::to_string(i) + "].linear",    light->linear);
        shader.SetFloat("pointLights[" + std::to_string(i) + "].quadratic", light->quadratic);
    }
    shader.SetInt("numPointLights", numPointLights);

    if (dirLight) {
        shader.SetVec3("dirLight.direction", dirLight->direction);
        shader.SetVec3("dirLight.ambient",  dirLight->color * 0.1f);
        shader.SetVec3("dirLight.diffuse",  dirLight->color * 0.8f);
        shader.SetVec3("dirLight.specular", dirLight->color * 1.0f);
        hasDirLight = true;
    } else {
        shader.SetVec3("dirLight.direction", glm::vec3(0,0,0));
        shader.SetVec3("dirLight.ambient",  glm::vec3(0,0,0));
        shader.SetVec3("dirLight.diffuse",  glm::vec3(0,0,0));
        shader.SetVec3("dirLight.specular", glm::vec3(0,0,0));
    }
}

void Renderer::RenderMeshes() {
    std::map<Mesh*, std::vector<glm::mat4>> meshGroups;
    for (auto& obj : scene.GetObjects()) {
        Mesh* mesh = nullptr;
        if (auto* asMesh = dynamic_cast<Mesh*>(obj.get())) {
            mesh = asMesh;
        } else if (auto* cube = dynamic_cast<Cube*>(obj.get())) {
            mesh = cube->GetMesh().get();
        } else if (auto* plane = dynamic_cast<Plane*>(obj.get())) {
            mesh = plane->GetMesh().get();
        } else if (auto* model = dynamic_cast<Model*>(obj.get())) {
            // Alle Meshes des Models rendern
            for (auto* modelMesh : model->GetMeshes()) {
                meshGroups[modelMesh].push_back(ComputeModelMatrix(*model));
            }
            continue; // Model ist kein Mesh, daher überspringen
        }
        if (mesh) {
            meshGroups[mesh].push_back(ComputeModelMatrix(*obj));
        }
    }
    for (auto& [mesh, matrices] : meshGroups) {
        if (!matrices.empty()) {
            mesh->SetModelMatrices(matrices);
            mesh->DrawInstanced(*shader);
        }
    }
}

void Renderer::InitializeGrid() {
    grid = std::make_unique<Grid>();
}

void Renderer::RenderGrid(float aspect) {
    if (grid) {
        grid->Render(camera, aspect);
    }
}

void Renderer::Render() {
    static int nextViewportWidth  = viewportWidth  > 0 ? viewportWidth  : 1;
    static int nextViewportHeight = viewportHeight > 0 ? viewportHeight : 1;

    while (!window.ShouldClose()) {
        double frameStart = glfwGetTime();
        deltaTime = frameStart - lastFrameTime;
        lastFrameTime = frameStart;

        UpdateFPS();
        Input();

        // Update Viewport FBO falls Größe sich geändert hat
        if (viewportFBO == 0 || nextViewportWidth != viewportWidth || nextViewportHeight != viewportHeight) {
            viewportWidth  = std::max(nextViewportWidth,  1);
            viewportHeight = std::max(nextViewportHeight, 1);
            CreateViewportFBO(viewportWidth, viewportHeight);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, viewportFBO);
        glViewport(0, 0, viewportWidth, viewportHeight);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float aspect = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);

        // Grid vor der Szene ohne Depth-Writes/-Test
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        RenderGrid(aspect);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        // Szene
        shader->Use();
        SetProjectionMatrix(camera.GetProjectionMatrix(aspect), camera.GetViewMatrix());
        SetMaterials();
        SetLighting(*shader);
        RenderMeshes();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        UpdateMeshCache();

        // UI
        ui.BeginFrame();

        // Viewport zeichnen
        ViewportRect rect = ui.DrawViewport(viewportTexture, viewportWidth, viewportHeight, scene);
        nextViewportWidth  = std::max(1, static_cast<int>(rect.size.x));
        nextViewportHeight = std::max(1, static_cast<int>(rect.size.y));

        ui.DrawAxisGizmo(camera.GetViewMatrix(), rect.pos, rect.size);
        ui.Draw(cachedMeshes, scene);
        ui.EndFrame();

        window.SwapBuffers();
        window.PollEvents();

        LimitFPS(frameStart, 300.0);
    }
    DeleteViewportFBO();
}