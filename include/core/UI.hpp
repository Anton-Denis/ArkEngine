#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"
#include <vector>
#include <map>
#include "Scene.hpp"
#include "../objects/Mesh.hpp"
#include "../objects/PointLight.hpp"
#include "../objects/DirectionalLight.hpp"
#include "../core/Window.hpp"
#include "../core/ProjectManager.hpp"
#include "../core/AxisGizmo.hpp"

struct ViewportRect {
    ImVec2 pos;  // absolute top-left of the image
    ImVec2 size; // image size in pixels
};

class UI {
public:
    UI(Window* windowObj, GLFWwindow* window);
    ~UI();

    void BeginFrame();
    void EndFrame();
    void Draw(const std::vector<Mesh*>& meshes, Scene& scene);
    ViewportRect DrawViewport(GLuint texture, int texWidth, int texHeight, Scene& scene);
    void DrawAxisGizmo(const glm::mat4& viewMatrix, ImVec2 imageAbsPos, ImVec2 imageSize);

    void DrawDirectoryTree();
    void DrawFileBrowser(Scene& scene);

    static std::string selectedFile;
    static bool showFileDialog;
    static std::filesystem::path selectedDir;

private:
    void DrawMainMenu(Scene& scene);
    void DrawSceneList(Scene& scene, int& selectedIndex);
    void DrawObjectInfo(Scene& scene, int selectedIndex, const std::vector<Mesh*>& meshes);
    void DrawStylingEditor();
    void SetStyle();
    void LoadStyle(const std::string& filename);
    void SaveStyle(const std::string& filename);
    void DrawDirectoryTreeRecursive(const std::filesystem::path& dir);
    void DrawDirectoryContextMenu(const std::filesystem::path& dir, bool isRoot);
    void DrawBreadcrumbs();

    static std::vector<std::filesystem::path> backHistory;
    static std::vector<std::filesystem::path> forwardHistory;



    std::map<std::string, std::string> fileIcons = {
            {".obj", "📦"},     // 3D Model
            {".fbx", "📦"},     // 3D Model
            {".gltf", "📦"},    // 3D Model
            {".png", "🖼️"},     // Image
            {".jpg", "🖼️"},     // Image
            {".jpeg", "🖼️"},    // Image
            {".bmp", "🖼️"},     // Image
            {".tga", "🖼️"},     // Image
            {".txt", "📄"},     // Text
            {".json", "⚙️"},    // Config
            {".shader", "🔧"},  // Shader
            {".glsl", "🔧"},    // Shader
            {".hlsl", "🔧"},    // Shader
            {".mtl", "🎨"},     // Material
            {"folder", "📁"},   // Ordner
            {"default", "📄"}   // Standard
    };

    std::string GetFileIcon(const std::filesystem::path& path);
    std::string GetFileSizeString(const std::filesystem::path& path);

    void DrawFileGrid(Scene& scene, const std::vector<std::filesystem::path>& folders,const std::vector<std::filesystem::path>& files,std::filesystem::path& renamingPath, char* renameBuffer, bool& startRename);

    GLuint folderIcon = 0;
    GLuint fileIcon = 0;
    GLuint folderOpenIcon = 0;


    void LoadIcons();

    GLFWwindow* window;
    Window* windowObj;
};