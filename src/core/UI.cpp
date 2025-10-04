#include "../../include/core/UI.hpp"
#include "../../include/objects/Cube.hpp"
#include "../../include/objects/Plane.hpp"
#include "glm/gtx/quaternion.hpp"
#include "ImGuiFileDialog.h"
#include <fstream>
#include <sstream>

std::filesystem::path UI::selectedDir;
std::string UI::selectedFile;
static std::filesystem::path parentDirForNewFolder;
static std::filesystem::path pathToRename;
static char renameName[128] = "";

UI::UI(Window* windowObj, GLFWwindow* window) : windowObj(windowObj), window(window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(window, false);  // ✅ FIX: Changed from true to false - don't install callbacks
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiIO& io = ImGui::GetIO();
    io.FontDefault = io.Fonts->AddFontFromFileTTF("resources/fonts/DMSans_36pt-Regular.ttf", 16.0f);
    SetStyle();
    LoadIcons();

    std::cout << "[UI] Initialized: ImGui callbacks NOT overriding InputSystem" << std::endl;
}

UI::~UI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UI::SetStyle() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    LoadStyle("style.txt");
}


void UI::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Reset per-frame states
    viewportClicked = false;
    viewportHoveredFrame = false;
    // Defocus viewport on any left click; will be re-focused in DrawViewport if click is inside
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        viewportFocused = false;
    }

    ImGuiWindowFlags window_flags = 0;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    window_flags |= ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), 0);

    ImGui::End();
}

void UI::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::Draw(const std::vector<Mesh*>& meshes, Scene& scene) {
    static int selectedIndex = 0;

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 oldPadding = style.FramePadding;
    style.FramePadding.y = 8.0f;

    DrawMainMenu(scene);

    style.FramePadding = oldPadding;

    DrawSceneList(scene, selectedIndex);
    DrawObjectInfo(scene, selectedIndex, meshes);
    DrawFileBrowser(scene);
    DrawStylingEditor();
}

void UI::DrawMainMenu(Scene& scene) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 oldPadding = style.FramePadding;
    style.FramePadding.y = 10.0f;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ImGui::MenuItem("Save");
            ImGui::MenuItem("Load");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::MenuItem("Undo", "Strg+Z");
            ImGui::MenuItem("Redo", "Strg+Y");
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    style.FramePadding = oldPadding;
}

void UI::DrawSceneList(Scene& scene, int& selectedIndex) {
    ImGui::Begin("Scene");
    ImGui::Text("Objects:");
    auto& objects = scene.GetObjects();
    std::map<std::string, int> typeCounter;
    bool anyItemHovered = false;

    for (int i = 0; i < objects.size(); ++i) {
        std::string label;
        if (dynamic_cast<Cube*>(objects[i].get()))
            label = "Cube";
        else if (dynamic_cast<Plane*>(objects[i].get()))
            label = "Plane";
        else if (dynamic_cast<Light*>(objects[i].get()))
            label = "Light";
        else
            label = "Unknown";
        int num = ++typeCounter[label];
        label += " " + std::to_string(num);
        bool selected = (selectedIndex == i);
        if (ImGui::Selectable(label.c_str(), selected)) {
            selectedIndex = i;
        }
        if (ImGui::IsItemHovered())
            anyItemHovered = true;

        if (ImGui::BeginPopupContextItem(("ObjectMenu" + std::to_string(i)).c_str())) {
            if (ImGui::MenuItem("Delete")) {
                scene.RemoveObjectAt(i);
                if (selectedIndex >= static_cast<int>(objects.size())) {
                    selectedIndex = static_cast<int>(objects.size()) - 1;
                }
                if (selectedIndex < 0) selectedIndex = 0;
            }
            ImGui::EndPopup();
        }
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL_PATH")) {
            const char* modelPath = (const char*)payload->Data;
            std::string pathStr(modelPath);

            std::string projectRoot = ProjectManager::Instance().GetProjectRoot();
            if (pathStr.find(projectRoot + "/assets/") != std::string::npos) {
                auto modelObject = std::make_shared<Model>(pathStr);
                scene.AddObject(modelObject);
            } else {
                auto modelObject = std::make_shared<Model>(pathStr);
                scene.AddObject(modelObject);
            }
            selectedIndex = static_cast<int>(scene.GetObjects().size()) - 1;
        }
        ImGui::EndDragDropTarget();
    }

    if (!anyItemHovered && ImGui::BeginPopupContextWindow("SceneContextMenu", ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::BeginMenu("Add GameObject")) {
            if (ImGui::MenuItem("Cube")) {
                scene.AddObject(std::make_unique<Cube>());
                selectedIndex = static_cast<int>(scene.GetObjects().size()) - 1;
            }
            if (ImGui::MenuItem("Plane")) {
                scene.AddObject(std::make_unique<Plane>());
                selectedIndex = static_cast<int>(scene.GetObjects().size()) - 1;
            }
            if (ImGui::MenuItem("PointLight")) {
                scene.AddObject(std::make_unique<PointLight>());
                selectedIndex = static_cast<int>(scene.GetObjects().size()) - 1;
            }
            if (ImGui::MenuItem("Directional Light")) {
                scene.AddObject(std::make_unique<DirectionalLight>());
                selectedIndex = static_cast<int>(scene.GetObjects().size()) - 1;
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void UI::LoadIcons() {
    folderIcon = ResourceManager::GetTexture("resources/icons/folder.png");
    fileIcon = ResourceManager::GetTexture("resources/icons/file.png");
}

void UI::DrawFileGrid(Scene& scene, const std::vector<std::filesystem::path>& folders,
                      const std::vector<std::filesystem::path>& files,
                      std::filesystem::path& renamingPath, char* renameBuffer, bool& startRename) {

    float itemSize = 80.0f;
    float spacing = 10.0f;
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();
    int columns = std::max(1, (int)((contentRegion.x + spacing) / (itemSize + spacing)));

    int currentColumn = 0;

    // Back-Button für übergeordneten Ordner hinzufügen
    ProjectManager& pm = ProjectManager::Instance();
    std::filesystem::path assetsRoot = std::filesystem::path(pm.GetProjectRoot()) / "assets";

    if (selectedDir != assetsRoot && selectedDir.has_parent_path()) {
        ImGui::BeginGroup();

        // Transparenter Hintergrund für Back-Button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        // Back-Icon als Button mit PNG
        if (folderIcon != 0) {
            if (ImGui::ImageButton("back_button", (void*)(intptr_t)folderIcon, ImVec2(itemSize, itemSize))) {
                selectedDir = selectedDir.parent_path();
            }
        } else {
            if (ImGui::Button("⬆️##back_button", ImVec2(itemSize, itemSize))) {
                selectedDir = selectedDir.parent_path();
            }
        }

        ImGui::PopStyleColor();

        // Label mit Zielordner-Namen
        ImGui::SetNextItemWidth(itemSize);
        std::string parentName = selectedDir.parent_path().filename().string();
        if (parentName.empty()) parentName = "assets";
        ImGui::TextWrapped("%s", parentName.c_str());

        ImGui::EndGroup();

        currentColumn++;
    }

    // Ordner anzeigen
    for (const auto& folder : folders) {
        if (currentColumn > 0 && currentColumn < columns) ImGui::SameLine();

        std::string folderName = folder.filename().string();

        ImGui::BeginGroup();

        // Transparenter Hintergrund für Ordner
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        // Icon als großer Button mit PNG
        if (folderIcon != 0) {
            std::string buttonId = "folder_" + folder.string();
            if (ImGui::ImageButton(buttonId.c_str(), (void*)(intptr_t)folderIcon, ImVec2(itemSize, itemSize))) {
                selectedDir = folder;
            }
        } else {
            // Fallback auf Emoji
            std::string icon = GetFileIcon(folder);
            if (ImGui::Button((icon + "##" + folder.string()).c_str(), ImVec2(itemSize, itemSize))) {
                selectedDir = folder;
            }
        }

        ImGui::PopStyleColor();

        // Doppelklick zum Öffnen
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            selectedDir = folder;
        }

        // Dateiname unten
        ImGui::SetNextItemWidth(itemSize);
        ImGui::TextWrapped("%s", folderName.c_str());

        ImGui::EndGroup();

        // Kontextmenü für Ordner mit eindeutiger ID
        if (ImGui::BeginPopupContextItem(("FolderContext_" + folder.string()).c_str())) {
            if (ImGui::MenuItem("New Folder")) {
                std::string newFolderName = "New Folder";
                int counter = 1;
                std::filesystem::path newPath = folder / newFolderName;

                while (std::filesystem::exists(newPath)) {
                    newPath = folder / (newFolderName + " " + std::to_string(counter));
                    counter++;
                }

                bool success = ProjectManager::Instance().CreateFolder(folder.string(), newPath.filename().string());
                if (!success) {
                    std::cerr << "Fehler beim Erstellen des Ordners!" << std::endl;
                }
            }
            if (ImGui::MenuItem("Rename")) {
                renamingPath = folder;
                strncpy(renameBuffer, folderName.c_str(), 255);
                renameBuffer[255] = '\0';
                startRename = true;
            }
            if (ImGui::MenuItem("Delete")) {
                try {
                    if (selectedDir == folder) {
                        selectedDir = selectedDir.parent_path();
                    }
                    std::filesystem::remove_all(folder);
                } catch (const std::filesystem::filesystem_error& e) {
                    std::cerr << "Fehler beim Löschen: " << e.what() << std::endl;
                }
            }
            ImGui::EndPopup();
        }

        currentColumn++;
        if (currentColumn >= columns) {
            currentColumn = 0;
        }
    }

    // Dateien anzeigen
    for (const auto& file : files) {
        if (currentColumn > 0) ImGui::SameLine();

        std::string fileName = file.filename().string();
        std::string icon = GetFileIcon(file);
        bool isSelected = (UI::selectedFile == file.string());

        ImGui::BeginGroup();

        // Icon mit PNG-Bild oder Fallback auf Emoji
        ImGui::PushStyleColor(ImGuiCol_Button, isSelected ? ImVec4(0.3f, 0.5f, 0.8f, 1.0f) : ImVec4(0, 0, 0, 0));

        if (fileIcon != 0) {
            // PNG-Icon verwenden
            std::string buttonId = "file_" + file.string();
            if (ImGui::ImageButton(buttonId.c_str(), (void*)(intptr_t)fileIcon, ImVec2(itemSize, itemSize))) {
                UI::selectedFile = file.string();
            }
        } else {
            // Fallback auf Emoji
            if (ImGui::Button((icon + "##" + file.string()).c_str(), ImVec2(itemSize, itemSize))) {
                UI::selectedFile = file.string();
            }
        }

        ImGui::PopStyleColor();

        // Drag & Drop für Models
        std::string extension = file.extension().string();
        if (extension == ".obj" && ImGui::BeginDragDropSource()) {
            std::string fullPath = file.string();
            ImGui::SetDragDropPayload("MODEL_PATH", fullPath.c_str(), fullPath.size() + 1);
            ImGui::Text("Model: %s", fileName.c_str());
            ImGui::EndDragDropSource();
        }

        // Dateiname und Größe
        ImGui::SetNextItemWidth(itemSize);
        ImGui::TextWrapped("%s", fileName.c_str());
        ImGui::SetNextItemWidth(itemSize);
        ImGui::TextWrapped("%s", GetFileSizeString(file).c_str());

        ImGui::EndGroup();

        // Kontextmenü für Dateien mit eindeutiger ID
        if (ImGui::BeginPopupContextItem(("FileContext_" + file.string()).c_str())) {
            if (ImGui::MenuItem("Rename")) {
                renamingPath = file;
                strncpy(renameBuffer, fileName.c_str(), 255);
                renameBuffer[255] = '\0';
                startRename = true;
            }
            if (ImGui::MenuItem("Delete")) {
                try {
                    std::filesystem::remove(file);
                    if (UI::selectedFile == file.string()) {
                        UI::selectedFile.clear();
                    }
                } catch (const std::filesystem::filesystem_error& e) {
                    std::cerr << "Fehler beim Löschen: " << e.what() << std::endl;
                }
            }
            ImGui::EndPopup();
        }

        currentColumn++;
        if (currentColumn >= columns) {
            currentColumn = 0;
        }
    }

    // Rename-Popup
    if (startRename) {
        ImGui::OpenPopup("RenameItem");
        startRename = false;
    }

    if (ImGui::BeginPopupModal("RenameItem", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Neuer Name:");
        ImGui::InputText("##rename", renameBuffer, 256);

        if (ImGui::Button("OK") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            try {
                std::filesystem::path newPath = renamingPath.parent_path() / renameBuffer;
                std::filesystem::rename(renamingPath, newPath);

                if (UI::selectedFile == renamingPath.string()) {
                    UI::selectedFile = newPath.string();
                }
                if (selectedDir == renamingPath) {
                    selectedDir = newPath;
                }
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Rename-Fehler: " << e.what() << std::endl;
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UI::DrawBreadcrumbs() {
    ProjectManager& pm = ProjectManager::Instance();
    std::filesystem::path projectRoot = pm.GetProjectRoot();
    std::filesystem::path assetsRoot = projectRoot / "assets";

    // Prüfen ob selectedDir gültig ist
    if (!std::filesystem::exists(selectedDir)) {
        selectedDir = assetsRoot;
    }

    std::filesystem::path relativePath = std::filesystem::relative(selectedDir, assetsRoot);

    // "assets" Button
    if (ImGui::Button("assets")) {
        selectedDir = assetsRoot;
    }

    // Pfad-Teile durchgehen
    std::filesystem::path currentPath = assetsRoot;

    for (const auto& part : relativePath) {
        if (part == ".") continue;

        ImGui::SameLine();
        ImGui::Text(">");
        ImGui::SameLine();

        currentPath /= part;
        std::string buttonLabel = part.string() + "##" + currentPath.string();
        if (ImGui::Button(buttonLabel.c_str())) {
            selectedDir = currentPath;
        }
    }
}

void UI::DrawObjectInfo(Scene& scene, int selectedIndex, const std::vector<Mesh*>& meshes) {
    auto& objects = scene.GetObjects();
    ImGui::Begin("Inspector");
    ImGui::Separator();
    ImGui::Text("Transform");
    if (!objects.empty() && selectedIndex >= 0 && selectedIndex < objects.size()) {
        auto& obj = objects[selectedIndex];

        glm::vec3 pos = obj->GetPosition();
        if (ImGui::DragFloat3("Position", &pos.x, 0.05f))
            obj->SetPosition(pos);

        glm::quat rot = obj->GetRotation();
        float quat[4] = { rot.w, rot.x, rot.y, rot.z };
        if (ImGui::DragFloat4("Rotation", quat, 0.01f)) {
            glm::quat newRot(quat[0], quat[1], quat[2], quat[3]);
            if (glm::length(newRot) > 1e-4f)
                obj->SetRotation(glm::normalize(newRot));
        }

        glm::vec3 scale = obj->GetScale();
        if (ImGui::DragFloat3("Scale", &scale.x, 0.05f, 0.01f, 100.0f))
            obj->SetScale(scale);

        // Licht-Inspector
        if (auto* light = dynamic_cast<Light*>(obj.get())) {
            ImGui::Separator();
            ImGui::Text("Light Optionen");
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
    } else {
        ImGui::Text("Kein Objekt vorhanden.");
    }
    ImGui::End();
}


ViewportRect UI::DrawViewport(GLuint texture, int /*texWidth*/, int /*texHeight*/, Scene& scene) {
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    // Verhindere 0x0
    ImVec2 imageSize = ImVec2(std::max(1.0f, avail.x), std::max(1.0f, avail.y));

    // aktuelle Cursor-Position innerhalb des Fensters (oben-links der Bildfläche)
    ImVec2 cursor = ImGui::GetCursorPos();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 absoluteViewportPos = ImVec2(windowPos.x + cursor.x, windowPos.y + cursor.y);

    // Interaktive Fläche für Drag&Drop über die ganze Bildfläche
    ImGui::InvisibleButton("viewport_dragdrop", imageSize);
    // Hover/Focus logic
    if (ImGui::IsItemHovered()) {
        viewportHoveredFrame = true;
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        viewportClicked = true;
        viewportFocused = true; // Focus when clicked
    }
    // Keep focus also if window itself gains focus via ImGui (e.g. click title bar)
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        // Do not override explicit defocus unless user clicked elsewhere
        if (viewportHoveredFrame) {
            viewportFocused = true;
        }
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL_PATH")) {
            const char* modelPath = (const char*)payload->Data;
            scene.AddObject(std::make_shared<Model>(std::string(modelPath)));
        }
        ImGui::EndDragDropTarget();
    }

    // Textur 1:1 auf die gesamte Fläche zeichnen (Flip Y beibehalten)
    ImGui::SetCursorPos(cursor);
    ImGui::Image((void*)(intptr_t)texture, imageSize, ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
    return { absoluteViewportPos, imageSize };
}


void UI::DrawAxisGizmo(const glm::mat4& viewMatrix,
                                  ImVec2 imageAbsPos, ImVec2 imageSize)
{
    const float gizmoSize = 120.0f;
    const float margin    = 16.0f;

    // Top-right INSIDE the image bounds
    ImVec2 gizmoPos = ImVec2(
            imageAbsPos.x + imageSize.x - gizmoSize - margin,
            imageAbsPos.y + margin
    );

    // Clip to the image rectangle so nothing spills out
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    dl->PushClipRect(imageAbsPos, ImVec2(imageAbsPos.x + imageSize.x,
                                         imageAbsPos.y + imageSize.y), true);

    GizmoColors colors;
    ImGui_DrawAxisDotsWithPosLines_Smooth(viewMatrix, gizmoPos, gizmoSize, colors);

    dl->PopClipRect();
}




void UI::DrawFileBrowser(Scene& scene) {
    static std::filesystem::path renamingPath;
    static char renameBuffer[256] = "";
    static bool startRename = false;
    static float splitterWidth = 250.0f; // Breite des linken Panels

    ImGui::Begin("Assets");

    // Splitter-Layout: Links Directory Tree, rechts Content Grid
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();

    // Linkes Panel: Directory Tree
    ImGui::BeginChild("DirectoryTree", ImVec2(splitterWidth, contentRegion.y), true);

    // Directory Tree Code hier einbetten
    static std::filesystem::path assetsRoot = std::filesystem::path(ProjectManager::Instance().GetProjectRoot()) / "assets";

    if (selectedDir.empty()) {
        selectedDir = assetsRoot;
    }

    // Root-Node
    ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_DefaultOpen;
    if (selectedDir == assetsRoot) {
        rootFlags |= ImGuiTreeNodeFlags_Selected;
    }
    bool rootOpen = ImGui::TreeNodeEx("assets", rootFlags);

    if (ImGui::IsItemClicked()) {
        selectedDir = assetsRoot;
    }

    // Kontextmenü für den Root-Ordner
    if (ImGui::BeginPopupContextItem("RootContext")) {
        if (ImGui::MenuItem("New Folder")) {
            std::string folderName = "New Folder";
            int counter = 1;
            std::filesystem::path newPath = assetsRoot / folderName;

            while (std::filesystem::exists(newPath)) {
                newPath = assetsRoot / (folderName + " " + std::to_string(counter));
                counter++;
            }

            bool success = ProjectManager::Instance().CreateFolder(assetsRoot.string(), newPath.filename().string());
            if (!success) {
                std::cerr << "Fehler beim Erstellen des Root-Ordners!" << std::endl;
            }
        }
        ImGui::EndPopup();
    }

    if (rootOpen) {
        DrawDirectoryTreeRecursive(assetsRoot);
        ImGui::TreePop();
    }

    // Kontextmenü für leeren Bereich im Directory Tree Panel
    if (ImGui::BeginPopupContextWindow("DirectoryTreeContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::MenuItem("New Folder")) {
            std::string folderName = "New Folder";
            int counter = 1;
            std::filesystem::path newPath = assetsRoot / folderName; // GEÄNDERT: Verwende assetsRoot statt selectedDir

            while (std::filesystem::exists(newPath)) {
                newPath = assetsRoot / (folderName + " " + std::to_string(counter)); // GEÄNDERT: Verwende assetsRoot
                counter++;
            }

            bool success = ProjectManager::Instance().CreateFolder(assetsRoot.string(), newPath.filename().string()); // GEÄNDERT: Verwende assetsRoot
            if (!success) {
                std::cerr << "Fehler beim Erstellen des Ordners im Directory Tree!" << std::endl;
            }
        }
        ImGui::EndPopup();
    }

    ImGui::EndChild();

    // Splitter
    ImGui::SameLine();
    ImGui::Button("##splitter", ImVec2(4.0f, contentRegion.y));
    if (ImGui::IsItemActive()) {
        splitterWidth += ImGui::GetIO().MouseDelta.x;
        splitterWidth = std::max(100.0f, std::min(splitterWidth, contentRegion.x - 100.0f));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }

    // Rechtes Panel: Content Grid
    ImGui::SameLine();
    ImGui::BeginChild("ContentGrid", ImVec2(0, contentRegion.y), true);

    // Navigation Breadcrumbs
    DrawBreadcrumbs();
    ImGui::Separator();

    // Content Grid
    try {
        if (std::filesystem::exists(selectedDir) && std::filesystem::is_directory(selectedDir)) {
            std::error_code ec;
            auto dirIter = std::filesystem::directory_iterator(selectedDir, ec);
            if (ec) {
                ImGui::Text("Fehler beim Öffnen des Verzeichnisses: %s", ec.message().c_str());
            } else {
                // Ordner und Dateien getrennt sammeln
                std::vector<std::filesystem::path> folders;
                std::vector<std::filesystem::path> files;

                for (const auto& entry : dirIter) {
                    if (entry.is_directory()) {
                        folders.push_back(entry.path());
                    } else {
                        files.push_back(entry.path());
                    }
                }

                // Sortieren
                std::sort(folders.begin(), folders.end());
                std::sort(files.begin(), files.end());

                // Grid-Ansicht
                DrawFileGrid(scene, folders, files, renamingPath, renameBuffer, startRename);
            }
        } else {
            ImGui::Text("Verzeichnis nicht gefunden oder ungültig");
        }
    } catch (const std::filesystem::filesystem_error& e) {
        ImGui::Text("Filesystem-Fehler: %s", e.what());
    }

    // Drag & Drop Import Logic
    extern std::vector<std::string> droppedFiles;
    if (!droppedFiles.empty()) {
        for (const auto& filePath : droppedFiles) {
            if (std::filesystem::path(filePath).extension() == ".obj") {
                std::filesystem::path projectRoot = ProjectManager::Instance().GetProjectRoot();
                std::filesystem::path assetsPath = projectRoot / "assets";
                if (filePath.find(assetsPath.string()) == std::string::npos) {
                    ProjectManager::Instance().ImportAsset(filePath, "model");
                }
            }
        }
        droppedFiles.clear();
    }

    ImGui::EndChild();

    ImGui::End();
}

void UI::DrawDirectoryTreeRecursive(const std::filesystem::path& dir) {
    static std::filesystem::path renamingPath;
    static char renameBuffer[256] = "";
    static bool startRename = false;

    try {
        if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
            return;
        }

        std::error_code ec;
        auto dirIter = std::filesystem::directory_iterator(dir, ec);
        if (ec) {
            std::cerr << "Fehler beim Öffnen des Verzeichnisses: " << dir << " - " << ec.message() << std::endl;
            return;
        }

        for (const auto& entry : dirIter) {
            if (entry.is_directory()) {
                std::string folderName = entry.path().filename().string();
                bool isRenaming = (renamingPath == entry.path());

                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
                if (selectedDir == entry.path()) {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }

                bool nodeOpen = false;

                if (isRenaming) {
                    // Inline-Bearbeitung
                    ImGui::SetNextItemWidth(-1);
                    if (startRename) {
                        ImGui::SetKeyboardFocusHere();
                        startRename = false;
                    }

                    if (ImGui::InputText(("##rename" + entry.path().string()).c_str(), renameBuffer, sizeof(renameBuffer),
                                         ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                        std::filesystem::path newPath = entry.path().parent_path() / renameBuffer;
                        if (!std::filesystem::exists(newPath) && strlen(renameBuffer) > 0) {
                            try {
                                std::filesystem::rename(entry.path(), newPath);
                                if (selectedDir == entry.path()) {
                                    selectedDir = newPath;
                                }
                            } catch (const std::filesystem::filesystem_error& e) {
                                std::cerr << "Fehler beim Umbenennen: " << e.what() << std::endl;
                            }
                        }
                        renamingPath.clear();
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_Escape) ||
                        (!ImGui::IsItemActive() && !ImGui::IsItemFocused() && ImGui::IsMouseClicked(0))) {
                        renamingPath.clear();
                    }
                } else {
                    // Normaler TreeNode
                    nodeOpen = ImGui::TreeNodeEx(folderName.c_str(), flags);

                    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                        selectedDir = entry.path();
                    }

                    // Kontextmenü für Directory Tree Ordner mit eindeutiger ID
                    if (ImGui::BeginPopupContextItem(("TreeFolderContext_" + entry.path().string()).c_str())) {
                        if (ImGui::MenuItem("New Folder")) {
                            std::string newFolderName = "New Folder";
                            int counter = 1;
                            std::filesystem::path newPath = entry.path() / newFolderName;

                            while (std::filesystem::exists(newPath)) {
                                newPath = entry.path() / (newFolderName + " " + std::to_string(counter));
                                counter++;
                            }

                            bool success = ProjectManager::Instance().CreateFolder(entry.path().string(), newPath.filename().string());
                            if (!success) {
                                std::cerr << "Fehler beim Erstellen des Ordners!" << std::endl;
                            }
                        }
                        if (ImGui::MenuItem("Rename")) {
                            renamingPath = entry.path();
                            strncpy(renameBuffer, folderName.c_str(), sizeof(renameBuffer) - 1);
                            renameBuffer[sizeof(renameBuffer) - 1] = '\0';
                            startRename = true;
                        }
                        if (ImGui::MenuItem("Delete")) {
                            if (std::filesystem::exists(entry.path())) {
                                if (selectedDir == entry.path()) {
                                    selectedDir = entry.path().parent_path();
                                }
                                try {
                                    std::filesystem::remove_all(entry.path());
                                } catch (const std::filesystem::filesystem_error& e) {
                                    std::cerr << "Fehler beim Löschen: " << e.what() << std::endl;
                                }
                            }
                        }
                        ImGui::EndPopup();
                    }
                }

                if (nodeOpen) {
                    DrawDirectoryTreeRecursive(entry.path());
                    ImGui::TreePop();
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem-Fehler in DrawDirectoryTreeRecursive: " << e.what() << std::endl;
        std::cerr << "Pfad: " << dir << std::endl;
    }
}

void UI::DrawDirectoryContextMenu(const std::filesystem::path& dir, bool isRoot) {
    if (!isRoot && ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("New Folder")) {
            std::string folderName = "New Folder";
            int counter = 1;
            std::filesystem::path newPath = dir / folderName;

            while (std::filesystem::exists(newPath)) {
                newPath = dir / (folderName + " " + std::to_string(counter));
                counter++;
            }

            std::cout << "Kontext-Ordner erstellen:" << std::endl;
            std::cout << "  Parent: " << dir.string() << std::endl;
            std::cout << "  Name: " << newPath.filename().string() << std::endl;

            bool success = ProjectManager::Instance().CreateFolder(dir.string(), newPath.filename().string());
            if (!success) {
                std::cerr << "Fehler beim Erstellen des Kontext-Ordners!" << std::endl;
            }
        }
        if (ImGui::MenuItem("Rename Folder")) {
            pathToRename = dir;
            std::string currentName = dir.filename().string();
            strncpy(renameName, currentName.c_str(), sizeof(renameName) - 1);
            renameName[sizeof(renameName) - 1] = '\0';
            ImGui::OpenPopup("RenameFolderPopup");
        }
        if (ImGui::MenuItem("Delete Folder")) {
            if (std::filesystem::exists(dir)) {
                if (selectedDir == dir) {
                    selectedDir = dir.parent_path();
                }
                std::filesystem::remove_all(dir);
            }
        }
        ImGui::EndPopup();
    }
}

std::string UI::GetFileIcon(const std::filesystem::path& path) {
    if (std::filesystem::is_directory(path)) {
        return fileIcons["folder"];
    }

    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    auto it = fileIcons.find(ext);
    return (it != fileIcons.end()) ? it->second : fileIcons["default"];
}

std::string UI::GetFileSizeString(const std::filesystem::path& path) {
    try {
        if (std::filesystem::is_directory(path)) {
            return "Folder";
        }

        auto fileSize = std::filesystem::file_size(path);
        if (fileSize < 1024) {
            return std::to_string(fileSize) + " B";
        } else if (fileSize < 1024 * 1024) {
            return std::to_string(fileSize / 1024) + " KB";
        } else {
            return std::to_string(fileSize / (1024 * 1024)) + " MB";
        }
    } catch (const std::filesystem::filesystem_error&) {
        return "Unknown";
    }
}

void UI::DrawStylingEditor() {
    ImGui::Begin("Styling");

    if (ImGui::Button("Style speichern")) {
        SaveStyle("style.txt");
    }
    ImGui::SameLine();
    if (ImGui::Button("Style laden")) {
        LoadStyle("style.txt");
    }

    ImGuiStyle& style = ImGui::GetStyle();

    ImGui::DragFloat("Alpha", &style.Alpha, 0.01f, 0.2f, 1.0f, "%.2f");
    ImGui::DragFloat2("WindowPadding", (float*)&style.WindowPadding, 0.1f, 0.0f, 30.0f, "%.1f");
    ImGui::DragFloat2("FramePadding", (float*)&style.FramePadding, 0.1f, 0.0f, 20.0f, "%.1f");
    ImGui::DragFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.1f, 0.0f, 20.0f, "%.1f");
    ImGui::DragFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.1f, 0.0f, 20.0f, "%.1f");
    ImGui::DragFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.1f, 0.0f, 10.0f, "%.1f");
    ImGui::DragFloat("IndentSpacing", &style.IndentSpacing, 0.1f, 0.0f, 30.0f, "%.1f");
    ImGui::DragFloat("ScrollbarSize", &style.ScrollbarSize, 0.1f, 5.0f, 30.0f, "%.1f");
    ImGui::DragFloat("GrabMinSize", &style.GrabMinSize, 0.1f, 1.0f, 20.0f, "%.1f");
    ImGui::DragFloat("WindowBorderSize", &style.WindowBorderSize, 0.01f, 0.0f, 5.0f, "%.2f");
    ImGui::DragFloat("ChildBorderSize", &style.ChildBorderSize, 0.01f, 0.0f, 5.0f, "%.2f");
    ImGui::DragFloat("PopupBorderSize", &style.PopupBorderSize, 0.01f, 0.0f, 5.0f, "%.2f");
    ImGui::DragFloat("FrameBorderSize", &style.FrameBorderSize, 0.01f, 0.0f, 5.0f, "%.2f");
    ImGui::DragFloat("TabBorderSize", &style.TabBorderSize, 0.01f, 0.0f, 5.0f, "%.2f");
    ImGui::DragFloat("WindowRounding", &style.WindowRounding, 0.1f, 0.0f, 20.0f, "%.1f");
    ImGui::DragFloat("ChildRounding", &style.ChildRounding, 0.1f, 0.0f, 20.0f, "%.1f");
    ImGui::DragFloat("FrameRounding", &style.FrameRounding, 0.1f, 0.0f, 20.0f, "%.1f");
    ImGui::DragFloat("PopupRounding", &style.PopupRounding, 0.1f, 0.0f, 20.0f, "%.1f");
    ImGui::DragFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.1f, 0.0f, 20.0f, "%.1f");
    ImGui::DragFloat("GrabRounding", &style.GrabRounding, 0.1f, 0.0f, 20.0f, "%.1f");
    ImGui::DragFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.01f, 0.0f, 10.0f, "%.2f");
    ImGui::DragFloat("TabRounding", &style.TabRounding, 0.1f, 0.0f, 20.0f, "%.1f");

    if (ImGui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (int i = 0; i < ImGuiCol_COUNT; ++i) {
            ImGui::ColorEdit4(ImGui::GetStyleColorName(i), (float*)&style.Colors[i]);
        }
    }

    if (ImGui::Button("Reset Style")) {
        ImGui::StyleColorsDark();
        SetStyle();
    }

    ImGui::End();
}

void UI::SaveStyle(const std::string& filename) {
    ImGuiStyle& style = ImGui::GetStyle();
    std::ofstream file(filename);
    if (!file) return;

    file << style.Alpha << "\n";
    file << style.WindowPadding.x << " " << style.WindowPadding.y << "\n";
    file << style.WindowRounding << "\n";
    file << style.WindowBorderSize << "\n";
    file << style.WindowMinSize.x << " " << style.WindowMinSize.y << "\n";
    file << style.WindowTitleAlign.x << " " << style.WindowTitleAlign.y << "\n";
    file << style.ChildRounding << "\n";
    file << style.ChildBorderSize << "\n";
    file << style.PopupRounding << "\n";
    file << style.PopupBorderSize << "\n";
    file << style.FramePadding.x << " " << style.FramePadding.y << "\n";
    file << style.FrameRounding << "\n";
    file << style.FrameBorderSize << "\n";
    file << style.ItemSpacing.x << " " << style.ItemSpacing.y << "\n";
    file << style.ItemInnerSpacing.x << " " << style.ItemInnerSpacing.y << "\n";
    file << style.TouchExtraPadding.x << " " << style.TouchExtraPadding.y << "\n";
    file << style.IndentSpacing << "\n";
    file << style.ColumnsMinSpacing << "\n";
    file << style.ScrollbarSize << "\n";
    file << style.ScrollbarRounding << "\n";
    file << style.GrabMinSize << "\n";
    file << style.GrabRounding << "\n";
    file << style.LogSliderDeadzone << "\n";
    file << style.TabRounding << "\n";
    file << style.TabBorderSize << "\n";
    // Farben
    for (int i = 0; i < ImGuiCol_COUNT; ++i) {
        ImVec4 c = style.Colors[i];
        file << c.x << " " << c.y << " " << c.z << " " << c.w << "\n";
    }
}

void UI::LoadStyle(const std::string& filename) {
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
    // Farben
    for (int i = 0; i < ImGuiCol_COUNT; ++i) {
        ImVec4& c = style.Colors[i];
        file >> c.x >> c.y >> c.z >> c.w;
    }
}
