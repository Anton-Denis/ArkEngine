#include "../../../include/core/ui/StyleEditorPanel.hpp"
#include "../../../include/core/ui/PanelContext.hpp"
#include "imgui.h"
#include <fstream>

void StyleEditorPanel::Draw(PanelContext&) {
    ImGui::Begin("Styling");
    if (ImGui::Button("Style speichern")) SaveStyle("style.txt");
    ImGui::SameLine();
    if (ImGui::Button("Style laden")) LoadStyle("style.txt");

    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::DragFloat("Alpha", &style.Alpha, 0.01f, 0.2f, 1.0f);
    ImGui::DragFloat2("WindowPadding", (float*)&style.WindowPadding, 0.1f, 0.0f, 30.0f);
    ImGui::DragFloat2("FramePadding", (float*)&style.FramePadding, 0.1f, 0.0f, 20.0f);
    ImGui::DragFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.1f, 0.0f, 20.0f);
    ImGui::DragFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.1f, 0.0f, 20.0f);
    ImGui::DragFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.1f, 0.0f, 10.0f);
    ImGui::DragFloat("IndentSpacing", &style.IndentSpacing, 0.1f, 0.0f, 30.0f);
    ImGui::DragFloat("ScrollbarSize", &style.ScrollbarSize, 0.1f, 5.0f, 30.0f);
    ImGui::DragFloat("GrabMinSize", &style.GrabMinSize, 0.1f, 1.0f, 20.0f);
    ImGui::DragFloat("WindowBorderSize", &style.WindowBorderSize, 0.01f, 0.0f, 5.0f);
    ImGui::DragFloat("ChildBorderSize", &style.ChildBorderSize, 0.01f, 0.0f, 5.0f);
    ImGui::DragFloat("PopupBorderSize", &style.PopupBorderSize, 0.01f, 0.0f, 5.0f);
    ImGui::DragFloat("FrameBorderSize", &style.FrameBorderSize, 0.01f, 0.0f, 5.0f);
    ImGui::DragFloat("TabBorderSize", &style.TabBorderSize, 0.01f, 0.0f, 5.0f);
    ImGui::DragFloat("WindowRounding", &style.WindowRounding, 0.1f, 0.0f, 20.0f);
    ImGui::DragFloat("ChildRounding", &style.ChildRounding, 0.1f, 0.0f, 20.0f);
    ImGui::DragFloat("FrameRounding", &style.FrameRounding, 0.1f, 0.0f, 20.0f);
    ImGui::DragFloat("PopupRounding", &style.PopupRounding, 0.1f, 0.0f, 20.0f);
    ImGui::DragFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.1f, 0.0f, 20.0f);
    ImGui::DragFloat("GrabRounding", &style.GrabRounding, 0.1f, 0.0f, 20.0f);
    ImGui::DragFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.01f, 0.0f, 10.0f);
    ImGui::DragFloat("TabRounding", &style.TabRounding, 0.1f, 0.0f, 20.0f);

    if (ImGui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (int i = 0; i < ImGuiCol_COUNT; ++i) {
            ImGui::ColorEdit4(ImGui::GetStyleColorName(i), (float*)&style.Colors[i]);
        }
    }

    if (ImGui::Button("Reset Style")) {
        ImGui::StyleColorsDark();
    }

    ImGui::End();
}

void StyleEditorPanel::SaveStyle(const char* filename) {
    ImGuiStyle& style = ImGui::GetStyle();
    std::ofstream file(filename);
    if (!file) return;
    file << style.Alpha << "\n";
    file << style.WindowPadding.x << " " << style.WindowPadding.y << "\n";
    file << style.WindowRounding << "\n" << style.WindowBorderSize << "\n";
    file << style.WindowMinSize.x << " " << style.WindowMinSize.y << "\n";
    file << style.WindowTitleAlign.x << " " << style.WindowTitleAlign.y << "\n";
    file << style.ChildRounding << "\n" << style.ChildBorderSize << "\n";
    file << style.PopupRounding << "\n" << style.PopupBorderSize << "\n";
    file << style.FramePadding.x << " " << style.FramePadding.y << "\n";
    file << style.FrameRounding << "\n" << style.FrameBorderSize << "\n";
    file << style.ItemSpacing.x << " " << style.ItemSpacing.y << "\n";
    file << style.ItemInnerSpacing.x << " " << style.ItemInnerSpacing.y << "\n";
    file << style.TouchExtraPadding.x << " " << style.TouchExtraPadding.y << "\n";
    file << style.IndentSpacing << "\n";
    file << style.ColumnsMinSpacing << "\n";
    file << style.ScrollbarSize << "\n" << style.ScrollbarRounding << "\n";
    file << style.GrabMinSize << "\n" << style.GrabRounding << "\n";
    file << style.LogSliderDeadzone << "\n";
    file << style.TabRounding << "\n" << style.TabBorderSize << "\n";
    for (int i = 0; i < ImGuiCol_COUNT; ++i) {
        ImVec4 c = style.Colors[i];
        file << c.x << " " << c.y << " " << c.z << " " << c.w << "\n";
    }
}

void StyleEditorPanel::LoadStyle(const char* filename) {
    ImGuiStyle& style = ImGui::GetStyle();
    std::ifstream file(filename);
    if (!file) return;
    file >> style.Alpha;
    file >> style.WindowPadding.x >> style.WindowPadding.y;
    file >> style.WindowRounding;
    file >> style.WindowBorderSize;
    file >> style.WindowMinSize.x >> style.WindowMinSize.y;
    file >> style.WindowTitleAlign.x >> style.WindowTitleAlign.y;
    file >> style.ChildRounding;
    file >> style.ChildBorderSize;
    file >> style.PopupRounding;
    file >> style.PopupBorderSize;
    file >> style.FramePadding.x >> style.FramePadding.y;
    file >> style.FrameRounding;
    file >> style.FrameBorderSize;
    file >> style.ItemSpacing.x >> style.ItemSpacing.y;
    file >> style.ItemInnerSpacing.x >> style.ItemInnerSpacing.y;
    file >> style.TouchExtraPadding.x >> style.TouchExtraPadding.y;
    file >> style.IndentSpacing;
    file >> style.ColumnsMinSpacing;
    file >> style.ScrollbarSize;
    file >> style.ScrollbarRounding;
    file >> style.GrabMinSize;
    file >> style.GrabRounding;
    file >> style.LogSliderDeadzone;
    file >> style.TabRounding;
    file >> style.TabBorderSize;
    for (int i = 0; i < ImGuiCol_COUNT; ++i) {
        ImVec4& c = style.Colors[i];
        file >> c.x >> c.y >> c.z >> c.w;
    }
}

