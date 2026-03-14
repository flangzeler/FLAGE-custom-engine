#include "menu_bar.h"
#include <imgui.h>
#include <windows.h>
#undef APIENTRY
#include <string>
#include <filesystem>
#include <shellapi.h>

// Engine Includes
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\Editor\GUI\Panels\project_manager\project_manager.h" 
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\importer\importer.h"
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\shaders\shaders_default.h"
namespace fs = std::filesystem;

// --- Native Dialog Helpers ---
static std::string OpenFileDialog(const wchar_t* filter) {
    OPENFILENAMEW ofn;
    wchar_t szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn) == TRUE) {
        char path[260];
        size_t converted = 0;
        wcstombs_s(&converted, path, sizeof(path), szFile, _TRUNCATE);
        return std::string(path);
    }
    return "";
}

static std::string SaveFileDialog(const wchar_t* filter) {
    OPENFILENAMEW ofn;
    wchar_t szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileNameW(&ofn) == TRUE) {
        char path[260];
        size_t converted = 0;
        wcstombs_s(&converted, path, sizeof(path), szFile, _TRUNCATE);
        return std::string(path);
    }
    return "";
}

MenuBar::MenuBar(Scene* scene) : scene(scene) {}

void MenuBar::Render() {
    if (!ImGui::BeginMainMenuBar()) return;

    // --- 1. FILE MENU ---
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
            scene->entities.clear();
            scene->world.entities.clear();
            // Clear specific components if necessary
        }
        Shader* shader = 0;
        if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) {
            std::string path = OpenFileDialog(L"FLAGE Scene (*.flagSCENE)\0*.flagSCENE\0");
            if (!path.empty()) ProjectManager::LoadScene(*scene, path, shader);
        }

        // --- DYNAMIC SCENE LIST ---
        if (ImGui::BeginMenu("Recent Scenes")) {
            if (ProjectManager::CurrentProject.scenePaths.empty()) {
                ImGui::TextDisabled("No scenes found in project");
            }
            else {
                for (const auto& sceneFile : ProjectManager::CurrentProject.scenePaths) {
                    if (ImGui::MenuItem(sceneFile.c_str())) {
                        std::string fullPath = ProjectManager::ToAbsolute("Scenes/" + sceneFile);
                        ProjectManager::LoadScene(*scene, fullPath,shader);
                    }
                }
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save All", "Ctrl+S")) {
            ProjectManager::SaveAll(*scene);
        }

        if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) {
            std::string path = SaveFileDialog(L"FLAGE Scene (*.flagSCENE)\0*.flagSCENE\0");
            if (!path.empty()) {
                std::string sceneName = fs::path(path).stem().string();
                ProjectManager::SaveActiveScene(*scene, sceneName);
            }
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Import")) {
            if (ImGui::MenuItem("3D Model Asset")) {
                std::string path = OpenFileDialog(L"Model Files\0*.obj;*.fbx;*.dae;*.gltf\0");
                if (!path.empty()) {
                    Entity e = Importer::ImportModel(*scene, path);
                    if (e.id != UINT32_MAX) {
                        scene->selectedEntity = e;
                        scene->selectedIndex = scene->FindIndexById(e.id);
                    }
                }
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Exit", "Alt+F4")) PostQuitMessage(0);

        ImGui::EndMenu();
    }

    // --- 2. EDIT MENU ---
    if (ImGui::BeginMenu("Edit")) {
        bool hasSelection = (scene->selectedIndex >= 0);

        if (ImGui::MenuItem("Undo", "Ctrl+Z", false, false)) { /* Undo Logic */ }
        if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) { /* Redo Logic */ }

        ImGui::Separator();

        if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, hasSelection)) {
            Entity dup = scene->DuplicateEntity(scene->selectedIndex);
            scene->selectedIndex = scene->FindIndexById(dup.id);
            scene->selectedEntity = dup;
        }

        if (ImGui::MenuItem("Delete", "Del", false, hasSelection)) {
            scene->DeleteEntity(scene->selectedIndex);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Create Prefab (.flagOBJ)", nullptr, false, hasSelection)) {
            Entity e = scene->entities[scene->selectedIndex];
            std::string name = scene->world.getName(e).name;
            ProjectManager::SaveEntityAsPrefab(*scene, e, name);
        }

        ImGui::EndMenu();
    }

    // --- 3. ASSETS & PROJECT ---
    if (ImGui::BeginMenu("Project")) {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Project: %s", ProjectManager::CurrentProject.projectName.c_str());
        ImGui::Separator();

        if (ImGui::MenuItem("Update Project Manifest")) {
            ProjectManager::UpdateProjectManifest();
        }

        if (ImGui::MenuItem("Open File Explorer")) {
            ShellExecuteA(NULL, "open", ProjectManager::CurrentProject.fullProjectPath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Close Project")) {
            ProjectManager::IsProjectLoaded = false;
        }

        ImGui::EndMenu();
    }

    // --- 4. VIEW / TOOLS ---
    if (ImGui::BeginMenu("Window")) {
        if (ImGui::MenuItem("Reset Layout")) { /* Logic to reset ImGui .ini */ }
        ImGui::Separator();
        // You can add toggles for your panels here
        static bool showConsole = true;
        ImGui::MenuItem("Console", nullptr, &showConsole);
        ImGui::EndMenu();
    }

    // --- 5. HELP ---
    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("About FLAGE Engine")) { /* Show Modal */ }
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}