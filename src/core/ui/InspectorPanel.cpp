#include "../../../include/core/ui/InspectorPanel.hpp"
#include "../../../include/core/ui/PanelContext.hpp"
#include "../../../include/core/Scene.hpp"
#include "../../../include/objects/GameObject.hpp"
#include "../../../include/objects/Light.hpp"
#include "imgui.h"
#include "glm/gtx/quaternion.hpp"

void InspectorPanel::Draw(PanelContext& ctx) {
    if (!ctx.scene || !ctx.selection) return;

    auto& objects = ctx.scene->GetObjects();
    ImGui::Begin("Inspector");
    if (objects.empty() || ctx.selection->selectedObject < 0 || ctx.selection->selectedObject >= (int)objects.size()) {
        ImGui::Text("Kein Objekt ausgewählt.");
        ImGui::End();
        return;
    }

    auto& obj = objects[ctx.selection->selectedObject];

    ImGui::Separator();
    ImGui::Text("Transform");

    // Position
    glm::vec3 pos = obj->GetPosition();
    if (ImGui::DragFloat3("Position", &pos.x, 0.05f)) obj->SetPosition(pos);

    // Rotation (Quaternion direkt)
    glm::quat rot = obj->GetRotation();
    float q[4] = { rot.w, rot.x, rot.y, rot.z };
    if (ImGui::DragFloat4("Rotation (quat)", q, 0.01f)) {
        glm::quat newRot(q[0], q[1], q[2], q[3]);
        if (glm::length(newRot) > 1e-4f) obj->SetRotation(glm::normalize(newRot));
    }

    // Scale
    glm::vec3 scale = obj->GetScale();
    if (ImGui::DragFloat3("Scale", &scale.x, 0.05f, 0.01f, 100.0f)) obj->SetScale(scale);

    // Light spezifische Attribute
    if (auto* light = dynamic_cast<Light*>(obj.get())) {
        ImGui::Separator();
        ImGui::Text("Light");
        ImGui::ColorEdit3("Color", (float*)&light->color);
        if (light->type == Light::Type::Directional) {
            ImGui::DragFloat3("Direction", (float*)&light->direction, 0.01f, -1.0f, 1.0f);
        }
        if (light->type == Light::Type::Point || light->type == Light::Type::Spot) {
            ImGui::DragFloat("Constant", &light->constant, 0.01f, 0.0f, 2.0f);
            ImGui::DragFloat("Linear", &light->linear, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Quadratic", &light->quadratic, 0.01f, 0.0f, 1.0f);
        }
        if (light->type == Light::Type::Spot) {
            ImGui::DragFloat("CutOff", &light->cutOff, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("OuterCutOff", &light->outerCutOff, 0.01f, 0.0f, 1.0f);
        }
    }

    ImGui::End();
}

