#include "../../../include/core/ui/MenuBarPanel.hpp"
#include "../../../include/core/ui/PanelContext.hpp"
#include "imgui.h"

void MenuBarPanel::Draw(PanelContext&) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 oldPadding = style.FramePadding;
    style.FramePadding.y = 10.0f;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save")) {
                // TODO: Szene speichern
            }
            if (ImGui::MenuItem("Load")) {
                // TODO: Szene laden
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::MenuItem("Undo", "Ctrl+Z", false, false);
            ImGui::MenuItem("Redo", "Ctrl+Y", false, false);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    style.FramePadding = oldPadding;
}

