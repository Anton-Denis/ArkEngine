#include "../../../include/core/ui/AssetBrowserPanel.hpp"
#include "../../../include/core/ui/PanelContext.hpp"
#include "../../../include/core/ProjectManager.hpp"
#include "../../../include/objects/Model.hpp"
#include "imgui.h"
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <cstring>

// Externe Drop-Liste (wie zuvor genutzt)
extern std::vector<std::string> droppedFiles;

AssetBrowserPanel::AssetBrowserPanel() {
    auto root = std::filesystem::path(ProjectManager::Instance().GetProjectRoot()) / "assets";
    if (std::filesystem::exists(root)) currentDir = root;
}

void AssetBrowserPanel::DrawBreadcrumbs(const std::filesystem::path& assetsRoot) {
    if (ImGui::Button("assets")) currentDir = assetsRoot;
    std::filesystem::path rel;
    try { rel = std::filesystem::relative(currentDir, assetsRoot); } catch(...) { rel.clear(); }
    std::filesystem::path accum = assetsRoot;
    for (auto& p : rel) {
        if (p == ".") continue;
        ImGui::SameLine(); ImGui::Text(">"); ImGui::SameLine();
        accum /= p;
        std::string btn = p.string()+"##"+accum.string();
        if (ImGui::Button(btn.c_str())) currentDir = accum;
    }
}

std::string AssetBrowserPanel::GetFileIcon(const std::filesystem::path& path) const {
    if (std::filesystem::is_directory(path)) return fileIcons.at("folder");
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    auto it = fileIcons.find(ext);
    return it!=fileIcons.end()? it->second : fileIcons.at("default");
}

std::string AssetBrowserPanel::GetFileSizeString(const std::filesystem::path& p) const {
    try {
        if (std::filesystem::is_directory(p)) return "Folder";
        auto sz = std::filesystem::file_size(p);
        if (sz < 1024) return std::to_string(sz)+" B";
        if (sz < 1024*1024) return std::to_string(sz/1024)+" KB";
        return std::to_string(sz/(1024*1024))+" MB";
    } catch(...) { return "?"; }
}

void AssetBrowserPanel::DrawDirectoryTreeRecursive(const std::filesystem::path& dir) {
    std::error_code ec; for (auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (ec) break; if (!entry.is_directory()) continue;
        auto path = entry.path(); std::string name = path.filename().string();
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
        if (path == currentDir) flags |= ImGuiTreeNodeFlags_Selected;
        bool open = ImGui::TreeNodeEx((name+"##"+path.string()).c_str(), flags);
        if (ImGui::IsItemClicked()) currentDir = path;
        if (ImGui::BeginPopupContextItem(("DirCtx"+path.string()).c_str())) {
            if (ImGui::MenuItem("Rename")) { renamingPath = path; strncpy(renameBuffer, name.c_str(), 255); startRename = true; }
            if (ImGui::MenuItem("Delete")) { try { if (currentDir == path) currentDir = path.parent_path(); std::filesystem::remove_all(path);} catch(...){} }
            ImGui::EndPopup();
        }
        if (open) { DrawDirectoryTreeRecursive(path); ImGui::TreePop(); }
    }
}

void AssetBrowserPanel::DrawDirectoryTree() {
    auto assetsRoot = std::filesystem::path(ProjectManager::Instance().GetProjectRoot()) / "assets";
    ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (currentDir == assetsRoot) rootFlags |= ImGuiTreeNodeFlags_Selected;
    bool open = ImGui::TreeNodeEx("assets", rootFlags);
    if (ImGui::IsItemClicked()) currentDir = assetsRoot;
    if (open) { DrawDirectoryTreeRecursive(assetsRoot); ImGui::TreePop(); }
}

void AssetBrowserPanel::DrawFileGrid(const std::vector<std::filesystem::path>& folders,
                                     const std::vector<std::filesystem::path>& files) {
    float itemSize = 80.0f; float spacing = 10.0f;
    int columns = std::max(1, (int)((ImGui::GetContentRegionAvail().x + spacing)/(itemSize+spacing)));
    int col = 0;

    for (auto& f : folders) {
        if (col>0) ImGui::SameLine();
        std::string name = f.filename().string();
        ImGui::BeginGroup();
        if (ImGui::Button((GetFileIcon(f)+"##"+f.string()).c_str(), ImVec2(itemSize,itemSize))) {
            currentDir = f; // open folder
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) currentDir = f;
        ImGui::TextWrapped("%s", name.c_str());
        ImGui::EndGroup();
        if (ImGui::BeginPopupContextItem(("FolderCtx"+f.string()).c_str())) {
            if (ImGui::MenuItem("Rename")) { renamingPath = f; strncpy(renameBuffer, name.c_str(), 255); startRename = true; }
            if (ImGui::MenuItem("Delete")) { try { if (currentDir==f) currentDir = currentDir.parent_path(); std::filesystem::remove_all(f);} catch(...){} }
            ImGui::EndPopup();
        }
        if (++col >= columns) col=0;
    }
    for (auto& file : files) {
        if (col>0) ImGui::SameLine();
        std::string fileName = file.filename().string();
        bool selected = (selectedFile == file.string());
        ImGui::BeginGroup();
        if (ImGui::Selectable((GetFileIcon(file)+"##"+file.string()).c_str(), selected, 0, ImVec2(itemSize,itemSize))) {
            selectedFile = file.string();
        }
        // Drag & Drop
        std::string ext = file.extension().string();
        if ((ext == ".obj" || ext == ".fbx" || ext == ".gltf") && ImGui::BeginDragDropSource()) {
            std::string full = file.string();
            ImGui::SetDragDropPayload("MODEL_PATH", full.c_str(), full.size()+1);
            ImGui::Text("%s", fileName.c_str());
            ImGui::EndDragDropSource();
        }
        ImGui::TextWrapped("%s", fileName.c_str());
        ImGui::TextDisabled("%s", GetFileSizeString(file).c_str());
        ImGui::EndGroup();
        if (ImGui::BeginPopupContextItem(("FileCtx"+file.string()).c_str())) {
            if (ImGui::MenuItem("Rename")) { renamingPath = file; strncpy(renameBuffer, fileName.c_str(),255); startRename = true; }
            if (ImGui::MenuItem("Delete")) { try { std::filesystem::remove(file); if (selectedFile==file.string()) selectedFile.clear(); } catch(...){} }
            ImGui::EndPopup();
        }
        if (++col >= columns) col=0;
    }

    if (startRename) { ImGui::OpenPopup("RenameAsset"); startRename = false; }
    if (ImGui::BeginPopupModal("RenameAsset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("##newname", renameBuffer, 256);
        if (ImGui::Button("OK")) {
            try {
                auto newPath = renamingPath.parent_path()/renameBuffer;
                if (std::string(renameBuffer).size()>0) {
                    std::filesystem::rename(renamingPath, newPath);
                    if (selectedFile == renamingPath.string()) selectedFile = newPath.string();
                    if (currentDir == renamingPath) currentDir = newPath;                }
            } catch (...) {}
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void AssetBrowserPanel::Draw(PanelContext&) {
    ImGui::Begin("Assets");
    auto assetsRoot = std::filesystem::path(ProjectManager::Instance().GetProjectRoot()) / "assets";
    if (!std::filesystem::exists(currentDir)) currentDir = assetsRoot;

    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("DirTree", ImVec2(splitterWidth, avail.y), true);
    DrawDirectoryTree();
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::Button("##split", ImVec2(4, avail.y));
    if (ImGui::IsItemActive()) {
        splitterWidth += ImGui::GetIO().MouseDelta.x;
        splitterWidth = std::clamp(splitterWidth, 100.0f, avail.x - 100.0f);
    }
    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

    ImGui::SameLine();
    ImGui::BeginChild("DirContent", ImVec2(0, avail.y), true);
    DrawBreadcrumbs(assetsRoot);
    ImGui::Separator();

    // Inhalte sammeln
    std::vector<std::filesystem::path> folders; std::vector<std::filesystem::path> files;
    try {
        if (std::filesystem::exists(currentDir) && std::filesystem::is_directory(currentDir)) {
            for (auto& entry : std::filesystem::directory_iterator(currentDir)) {
                if (entry.is_directory()) folders.push_back(entry.path()); else files.push_back(entry.path());
            }
            std::sort(folders.begin(), folders.end());
            std::sort(files.begin(), files.end());
        }
    } catch (const std::filesystem::filesystem_error& e) { ImGui::Text("FS Error: %s", e.what()); }

    DrawFileGrid(folders, files);

    // Import über extern gedroppte Dateien
    if (!droppedFiles.empty()) {
        for (auto& p : droppedFiles) {
            std::filesystem::path path(p);
            if (path.extension() == ".obj") {
                // Falls nicht im assets Pfad -> importieren
                if (p.find(assetsRoot.string()) == std::string::npos) {
                    ProjectManager::Instance().ImportAsset(p, "model");
                }
            }
        }
        droppedFiles.clear();
    }
    ImGui::EndChild();
    ImGui::End();
}
