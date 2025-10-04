#include "../../include/core/UI.hpp"
#include "../../include/core/ui/IPanel.hpp"
#include "../../include/core/ProjectManager.hpp"
#include "../../include/core/ui/PanelContext.hpp"
#include "../../include/core/ui/MenuBarPanel.hpp"
#include "../../include/core/ui/SceneHierarchyPanel.hpp"
#include "../../include/core/ui/InspectorPanel.hpp"
#include "../../include/core/ui/AssetBrowserPanel.hpp"
#include "../../include/core/ui/StyleEditorPanel.hpp"
#include "../../include/core/ui/MonitoringPanel.hpp"
#include "../../include/objects/Model.hpp"
#include <iostream>

UI::UI(Window* windowObj, GLFWwindow* window) : windowObj(windowObj), window(window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiIO& io = ImGui::GetIO();
    io.FontDefault = io.Fonts->AddFontFromFileTTF("resources/fonts/DMSans_36pt-Regular.ttf", 16.0f);
    SetStyle();
    LoadIcons();

    // Panels registrieren
    panels.emplace_back(std::make_unique<MenuBarPanel>());
    panels.emplace_back(std::make_unique<SceneHierarchyPanel>());
    panels.emplace_back(std::make_unique<InspectorPanel>());
    panels.emplace_back(std::make_unique<AssetBrowserPanel>());
    panels.emplace_back(std::make_unique<StyleEditorPanel>());
    panels.emplace_back(std::make_unique<MonitoringPanel>());

    std::cout << "[UI] Initialized (refactored panel system)" << std::endl;
}

UI::~UI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UI::SetStyle() {
    ImGui::StyleColorsDark();
    // Optional: später Style-Manager einbauen
}

void UI::LoadIcons() {
    // Früher: folderIcon = ResourceManager::GetTexture(...)
    // Placeholder: lassen 0 falls nicht benötigt; AssetBrowserPanel lädt eigene Icons.
}

void UI::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    viewportClicked = false;
    viewportHoveredFrame = false;
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        viewportFocused = false;
    }

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::Begin("DockSpace", nullptr, flags);
    ImGui::PopStyleVar(3);
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0,0), 0);
    ImGui::End();
}

void UI::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::Draw(const std::vector<Mesh*>& meshes, Scene& scene) {
    PanelContext ctx{ &scene, &meshes, &selectionState };
    for (auto& p : panels) {
        p->Draw(ctx);
    }
}

ViewportRect UI::DrawViewport(GLuint texture, int, int, Scene& scene) {
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImVec2 imageSize(std::max(1.0f, avail.x), std::max(1.0f, avail.y));

    ImVec2 cursor = ImGui::GetCursorPos();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 absolutePos(windowPos.x + cursor.x, windowPos.y + cursor.y);

    ImGui::InvisibleButton("viewport_dragdrop", imageSize);
    if (ImGui::IsItemHovered()) viewportHoveredFrame = true;
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) { viewportClicked = true; viewportFocused = true; }
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && viewportHoveredFrame) viewportFocused = true;

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL_PATH")) {
            const char* modelPath = (const char*)payload->Data;
            scene.AddObject(std::make_shared<Model>(std::string(modelPath)));
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::SetCursorPos(cursor);
    ImGui::Image((void*)(intptr_t)texture, imageSize, ImVec2(0,1), ImVec2(1,0));
    ImGui::End();
    return { absolutePos, imageSize };
}

void UI::DrawAxisGizmo(const glm::mat4& viewMatrix, ImVec2 imageAbsPos, ImVec2 imageSize) {
    const float gizmoSize = 120.0f; const float margin = 16.0f;
    ImVec2 gizmoPos(imageAbsPos.x + imageSize.x - gizmoSize - margin, imageAbsPos.y + margin);
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    dl->PushClipRect(imageAbsPos, ImVec2(imageAbsPos.x + imageSize.x, imageAbsPos.y + imageSize.y), true);
    GizmoColors colors; // Standardfarben
    ImGui_DrawAxisDotsWithPosLines_Smooth(viewMatrix, gizmoPos, gizmoSize, colors);
    dl->PopClipRect();
}
