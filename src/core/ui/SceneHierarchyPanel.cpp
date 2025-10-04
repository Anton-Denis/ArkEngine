#include "../../../include/core/ui/SceneHierarchyPanel.hpp"
#include "../../../include/core/ui/PanelContext.hpp"
#include "../../../include/core/Scene.hpp"
#include "../../../include/objects/Cube.hpp"
#include "../../../include/objects/Plane.hpp"
#include "../../../include/objects/Model.hpp"
#include "../../../include/objects/PointLight.hpp"
#include "../../../include/objects/DirectionalLight.hpp"
#include "../../../include/objects/SpotLight.hpp"
#include "imgui.h"
#include <map>

void SceneHierarchyPanel::Draw(PanelContext& ctx) {
    if (!ctx.scene || !ctx.selection) return;
    ImGui::Begin("Scene");
    ImGui::Text("Objects:");
    auto& objects = ctx.scene->GetObjects();
    std::map<std::string,int> typeCounter;

    for (int i=0;i< (int)objects.size();++i) {
        std::string label;
        if (dynamic_cast<Cube*>(objects[i].get())) label = "Cube"; else
        if (dynamic_cast<Plane*>(objects[i].get())) label = "Plane"; else
        if (dynamic_cast<Light*>(objects[i].get())) label = "Light"; else
        if (dynamic_cast<Model*>(objects[i].get())) label = "Model"; else label = "Unknown";
        int num = ++typeCounter[label];
        label += " " + std::to_string(num);
        bool selected = (ctx.selection->selectedObject == i);
        if (ImGui::Selectable(label.c_str(), selected)) {
            ctx.selection->selectedObject = i;
        }
        if (ImGui::BeginPopupContextItem(("ObjectMenu" + std::to_string(i)).c_str())) {
            if (ImGui::MenuItem("Delete")) {
                ctx.scene->RemoveObjectAt(i);
                if (ctx.selection->selectedObject >= (int)objects.size()) {
                    ctx.selection->selectedObject = (int)objects.size() - 1;
                }
                if (ctx.selection->selectedObject < 0) ctx.selection->selectedObject = 0;
            }
            ImGui::EndPopup();
        }
    }

    if (ImGui::BeginPopupContextWindow("SceneContextMenu", ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::BeginMenu("Add GameObject")) {
            if (ImGui::MenuItem("Cube")) {
                ctx.scene->AddObject(std::make_unique<Cube>());
                ctx.selection->selectedObject = (int)ctx.scene->GetObjects().size()-1;
            }
            if (ImGui::MenuItem("Plane")) {
                ctx.scene->AddObject(std::make_unique<Plane>());
                ctx.selection->selectedObject = (int)ctx.scene->GetObjects().size()-1;
            }
            if (ImGui::MenuItem("PointLight")) {
                ctx.scene->AddObject(std::make_unique<PointLight>());
                ctx.selection->selectedObject = (int)ctx.scene->GetObjects().size()-1;
            }
            if (ImGui::MenuItem("Directional Light")) {
                ctx.scene->AddObject(std::make_unique<DirectionalLight>());
                ctx.selection->selectedObject = (int)ctx.scene->GetObjects().size()-1;
            }
            if (ImGui::MenuItem("Spot Light")) {
                ctx.scene->AddObject(std::make_unique<SpotLight>());
                ctx.selection->selectedObject = (int)ctx.scene->GetObjects().size()-1;
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}
