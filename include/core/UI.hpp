#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"
#include <vector>
#include <memory>
#include <filesystem>
#include "Scene.hpp"
#include "../objects/Mesh.hpp"
#include "../objects/PointLight.hpp"
#include "../objects/DirectionalLight.hpp"
#include "../core/Window.hpp"
#include "../core/ProjectManager.hpp"
#include "../core/AxisGizmo.hpp"
#include "ui/IPanel.hpp"
#include "ui/PanelContext.hpp"

struct ViewportRect { ImVec2 pos; ImVec2 size; };

class UI {
public:
    UI(Window* windowObj, GLFWwindow* window);
    ~UI();

    void BeginFrame();
    void EndFrame();

    // Zeichnet alle registrierten Panels (ohne Viewport/Achsen-Gizmo)
    void Draw(const std::vector<Mesh*>& meshes, Scene& scene);

    // Viewport separat (Renderer ruft das auf)
    ViewportRect DrawViewport(GLuint texture, int texWidth, int texHeight, Scene& scene);
    void DrawAxisGizmo(const glm::mat4& viewMatrix, ImVec2 imageAbsPos, ImVec2 imageSize);

    bool IsViewportClicked() const { return viewportClicked; }
    bool IsViewportFocused() const { return viewportFocused; }
    bool IsViewportHovered() const { return viewportHoveredFrame; }

    SelectionState& GetSelectionState() { return selectionState; }

private:
    void SetStyle();
    void LoadIcons();

    // ImGui Zustand
    bool viewportClicked = false;
    bool viewportFocused = false;
    bool viewportHoveredFrame = false;

    // Icons (für Panels nutzbar via Freundschaft falls nötig)
    GLuint folderIcon = 0;
    GLuint fileIcon = 0;

    // Panel-Verwaltung
    std::vector<std::unique_ptr<IPanel>> panels;
    SelectionState selectionState; // nutzt Definition aus PanelContext.hpp
    GLFWwindow* window;
    Window* windowObj;
};