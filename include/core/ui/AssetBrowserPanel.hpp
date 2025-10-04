#pragma once
#include "IPanel.hpp"
#include <filesystem>
#include <vector>
#include <map>
#include <string>
#include <memory>

class AssetBrowserPanel : public IPanel {
public:
    AssetBrowserPanel();
    const char* Name() const override { return "AssetBrowser"; }
    void Draw(PanelContext& ctx) override;
private:
    // State
    std::filesystem::path currentDir;
    std::string selectedFile;
    std::filesystem::path renamingPath;
    bool startRename = false;
    float splitterWidth = 250.0f;

    // Helpers
    void DrawDirectoryTree();
    void DrawDirectoryTreeRecursive(const std::filesystem::path& dir);
    void DrawBreadcrumbs(const std::filesystem::path& assetsRoot);
    void DrawFileGrid(const std::vector<std::filesystem::path>& folders,
                      const std::vector<std::filesystem::path>& files);
    std::string GetFileIcon(const std::filesystem::path& path) const;
    std::string GetFileSizeString(const std::filesystem::path& path) const;

    // Icons (optional texture IDs)
    unsigned int folderIcon = 0;
    unsigned int fileIcon = 0;

    // Rename popup buffer
    char renameBuffer[256] = {0};

    std::map<std::string, std::string> fileIcons = {
            {".obj", ""}, {".fbx", ""}, {".gltf", ""},
            {".png", ""}, {".jpg", ""}, {".jpeg", ""}, {".bmp", ""}, {".tga", ""},
            {".txt", ""}, {".json", "⚙"}, {".shader", ""}, {".glsl", ""}, {".hlsl", ""}, {".mtl", ""},
            {"folder", ""}, {"default", ""}
    };
};

