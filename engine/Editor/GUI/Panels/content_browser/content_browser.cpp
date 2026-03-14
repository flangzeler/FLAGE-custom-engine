#include "content_browser.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\Editor\GUI\Panels\project_manager\project_manager.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\importer\importer.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\scene\scene_class\scene.h"

#include <imgui.h>
#include <glad/glad.h>
#include <algorithm>
#include <iostream>
#include <fstream>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "E:/PROJECTS/glfw/learning/project_FLAGE/Libraries/include/STB/stb_image.h"
#endif

ContentBrowser::ContentBrowser() {
    memset(m_SearchBuffer, 0, sizeof(m_SearchBuffer));
    memset(m_RenameBuffer, 0, sizeof(m_RenameBuffer));
    memset(m_NewFileNameBuffer, 0, sizeof(m_NewFileNameBuffer));

    // Load icons
    m_IconFolder = LoadTexture("resources/icons/folder.png");
    m_IconFile = LoadTexture("resources/icons/generic file.png");
    m_IconModel = LoadTexture("resources/icons/FBX.png");
    m_IconCode = LoadTexture("resources/icons/code.png");
    m_IconImage = LoadTexture("resources/icons/image.png");
    m_IconScene = LoadTexture("resources/icons/generic file.png");
    m_IconMaterial = LoadTexture("resources/icons/generic file.png");
    m_IconPrefab = LoadTexture("resources/icons/generic file.png");
}

ContentBrowser::~ContentBrowser() {
    unsigned int texs[] = {
        m_IconFolder, m_IconFile, m_IconModel, m_IconCode,
        m_IconImage, m_IconScene, m_IconMaterial, m_IconPrefab
    };
    glDeleteTextures(8, texs);
}

void ContentBrowser::SetProjectRoot(const std::string& projectPath) {
    m_ProjectRoot = fs::path(projectPath);
    m_CurrentPath = m_ProjectRoot;

    // Clear navigation stacks when changing projects
    while (!m_BackStack.empty()) m_BackStack.pop();
    while (!m_ForwardStack.empty()) m_ForwardStack.pop();
}

void ContentBrowser::RefreshCurrentDirectory() {
    // Force a refresh - ImGui will handle it on next render
}

unsigned int ContentBrowser::LoadTexture(const char* path) {
    int w, h, ch;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(path, &w, &h, &ch, 4);

    if (!data) {
        std::cout << "[ContentBrowser] Warning: Could not load icon " << path << std::endl;
        return 0;
    }

    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return id;
}

void ContentBrowser::ChangeDirectory(const fs::path& newPath) {
    // Prevent navigating outside project root
    if (!newPath.string().find(m_ProjectRoot.string()) == 0) {
        return;
    }

    if (m_CurrentPath != newPath) {
        m_BackStack.push(m_CurrentPath);
        m_CurrentPath = newPath;
        while (!m_ForwardStack.empty()) m_ForwardStack.pop();
    }
}

void ContentBrowser::NavigateUp() {
    if (m_CurrentPath != m_ProjectRoot) {
        ChangeDirectory(m_CurrentPath.parent_path());
    }
}

unsigned int ContentBrowser::GetIconForPath(const fs::path& path) {
    if (fs::is_directory(path)) {
        return m_IconFolder;
    }

    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // Project-specific files
    if (ext == ".flagscene") return m_IconScene;
    if (ext == ".flagmat") return m_IconMaterial;
    if (ext == ".flagobj") return m_IconPrefab;

    // Standard files
    if (ext == ".fbx" || ext == ".obj" || ext == ".gltf" || ext == ".glb") return m_IconModel;
    if (ext == ".cpp" || ext == ".h" || ext == ".hpp" || ext == ".c" || ext == ".glsl") return m_IconCode;
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga" || ext == ".hdr") return m_IconImage;

    return m_IconFile;
}

bool ContentBrowser::PassesFilter(const fs::path& path) {
    if (fs::is_directory(path)) return true;

    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    switch (m_CurrentFilter) {
    case FileFilter::All: return true;
    case FileFilter::Models:
        return ext == ".fbx" || ext == ".obj" || ext == ".gltf" || ext == ".glb";
    case FileFilter::Images:
        return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga" || ext == ".hdr";
    case FileFilter::Scenes:
        return ext == ".flagscene";
    case FileFilter::Scripts:
        return ext == ".cpp" || ext == ".h" || ext == ".hpp" || ext == ".c";
    case FileFilter::Materials:
        return ext == ".flagmat";
    }
    return true;
}

std::string ContentBrowser::GetFileSize(const fs::path& path) {
    if (fs::is_directory(path)) return "Folder";

    try {
        auto size = fs::file_size(path);
        if (size < 1024) return std::to_string(size) + " B";
        if (size < 1024 * 1024) return std::to_string(size / 1024) + " KB";
        return std::to_string(size / (1024 * 1024)) + " MB";
    }
    catch (...) {
        return "Unknown";
    }
}

bool ContentBrowser::IsProjectFile(const std::string& extension) {
    return extension == ".flagscene" || extension == ".flagmat" ||
        extension == ".flagobj" || extension == ".flagproj";
}

void ContentBrowser::DeletePath(const fs::path& path) {
    try {
        if (fs::is_directory(path)) {
            fs::remove_all(path);
        }
        else {
            fs::remove(path);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ContentBrowser] Delete failed: " << e.what() << std::endl;
    }
}

void ContentBrowser::CopyPath(const fs::path& path) {
    m_CopiedPath = path;
    m_IsCutOperation = false;
}

void ContentBrowser::CutPath(const fs::path& path) {
    m_CopiedPath = path;
    m_IsCutOperation = true;
}

void ContentBrowser::PasteToCurrentDirectory() {
    if (m_CopiedPath.empty()) return;

    try {
        fs::path destination = m_CurrentPath / m_CopiedPath.filename();

        if (m_IsCutOperation) {
            fs::rename(m_CopiedPath, destination);
            m_CopiedPath.clear();
        }
        else {
            if (fs::is_directory(m_CopiedPath)) {
                fs::copy(m_CopiedPath, destination, fs::copy_options::recursive);
            }
            else {
                fs::copy_file(m_CopiedPath, destination);
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ContentBrowser] Paste failed: " << e.what() << std::endl;
    }
}

void ContentBrowser::RenamePath(const fs::path& oldPath, const std::string& newName) {
    try {
        fs::path newPath = oldPath.parent_path() / newName;
        fs::rename(oldPath, newPath);
    }
    catch (const std::exception& e) {
        std::cerr << "[ContentBrowser] Rename failed: " << e.what() << std::endl;
    }
}

void ContentBrowser::CreateNewFolder(const std::string& name) {
    try {
        fs::create_directory(m_CurrentPath / name);
    }
    catch (const std::exception& e) {
        std::cerr << "[ContentBrowser] Create folder failed: " << e.what() << std::endl;
    }
}

void ContentBrowser::RenderBreadcrumbs() {
    // Build breadcrumb path
    std::vector<fs::path> breadcrumbs;
    fs::path temp = m_CurrentPath;

    while (temp != m_ProjectRoot && temp.has_parent_path()) {
        breadcrumbs.push_back(temp);
        temp = temp.parent_path();
    }
    breadcrumbs.push_back(m_ProjectRoot);
    std::reverse(breadcrumbs.begin(), breadcrumbs.end());

    // Render breadcrumbs
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.3f, 0.4f, 1.0f));
    for (size_t i = 0; i < breadcrumbs.size(); ++i) {
        std::string label = (i == 0) ? "[Project]" : breadcrumbs[i].filename().string();

        if (ImGui::SmallButton(label.c_str())) {
            ChangeDirectory(breadcrumbs[i]);
        }

        if (i < breadcrumbs.size() - 1) {
            ImGui::SameLine();
            ImGui::TextDisabled("/");
            ImGui::SameLine();
        }
    }
    ImGui::PopStyleColor();
}

void ContentBrowser::RenderToolbar() {
    // Navigation buttons
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));

    if (ImGui::Button("<##Back") && !m_BackStack.empty()) {
        m_ForwardStack.push(m_CurrentPath);
        m_CurrentPath = m_BackStack.top();
        m_BackStack.pop();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Back");

    ImGui::SameLine();
    if (ImGui::Button(">##Forward") && !m_ForwardStack.empty()) {
        m_BackStack.push(m_CurrentPath);
        m_CurrentPath = m_ForwardStack.top();
        m_ForwardStack.pop();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Forward");

    ImGui::SameLine();
    bool canGoUp = (m_CurrentPath != m_ProjectRoot);
    if (!canGoUp) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    }
    if (ImGui::Button("^##Up")) {
        NavigateUp();
    }
    if (!canGoUp) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
    if (ImGui::IsItemHovered() && canGoUp) ImGui::SetTooltip("Up");

    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        RefreshCurrentDirectory();
    }

    ImGui::PopStyleVar();

    // Search bar
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputTextWithHint("##Search", "Search files...", m_SearchBuffer, 256);

    // Icon size slider
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.0f);
    ImGui::SliderFloat("##IconSize", &m_ThumbnailSize, 40.0f, 160.0f, "%.0f");
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Icon Size");

    // Filter dropdown
    ImGui::SameLine();
    const char* filterNames[] = { "All Files", "Models", "Images", "Scenes", "Scripts", "Materials" };
    int currentFilter = (int)m_CurrentFilter;
    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::Combo("##Filter", &currentFilter, filterNames, 6)) {
        m_CurrentFilter = (FileFilter)currentFilter;
    }
}

void ContentBrowser::RenderFileGrid() {
    // Calculate grid columns
    float cellSize = m_ThumbnailSize + 16.0f;
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = std::max(1, (int)(panelWidth / cellSize));

    ImGui::Columns(columnCount, 0, false);

    // Collect and sort entries
    std::vector<fs::directory_entry> entries;
    try {
        for (auto& entry : fs::directory_iterator(m_CurrentPath)) {
            entries.push_back(entry);
        }
    }
    catch (const std::exception& e) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error reading directory!");
        ImGui::Columns(1);
        return;
    }

    // Sort: folders first, then alphabetically
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        if (a.is_directory() != b.is_directory()) return a.is_directory();
        return a.path().filename().string() < b.path().filename().string();
        });

    // Render entries
    for (auto& entry : entries) {
        const auto& path = entry.path();
        std::string filename = path.filename().string();

        // Hidden files filter
        if (!m_ShowHiddenFiles && filename[0] == '.') continue;

        // Search filter
        if (m_SearchBuffer[0] != '\0') {
            std::string search = m_SearchBuffer;
            std::string lowerName = filename;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            std::transform(search.begin(), search.end(), search.begin(), ::tolower);
            if (lowerName.find(search) == std::string::npos) continue;
        }

        // Type filter
        if (!PassesFilter(path)) continue;

        ImGui::PushID(filename.c_str());

        // Get icon
        unsigned int icon = GetIconForPath(path);

        // Selection highlight
        bool isSelected = (m_SelectedPath == path);
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 0.6f));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        }

        // Image button
        ImGui::ImageButton(
            filename.c_str(),
            (ImTextureID)(uintptr_t)icon,
            ImVec2(m_ThumbnailSize, m_ThumbnailSize),
            ImVec2(0, 1), ImVec2(1, 0)
        );

        ImGui::PopStyleColor();

        // Single click selection
        if (ImGui::IsItemClicked()) {
            m_SelectedPath = path;
        }

        // Double click navigation/open
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            if (entry.is_directory()) {
                ChangeDirectory(path);
            }
            else {
                // Handle different file types
                std::string ext = path.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == ".flagscene") {
                    LoadScene(path);
                }
                else if (ext == ".flagobj") {
                    LoadPrefab(path);
                }
                else if (ext == ".fbx" || ext == ".obj" || ext == ".gltf" || ext == ".glb") {
                    ImportModel(path);
                }
                else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") {
                    OpenFileInExternalEditor(path);
                }
                else if (ext == ".cpp" || ext == ".h" || ext == ".glsl") {
                    OpenFileInExternalEditor(path);
                }
                else {
                    std::cout << "[ContentBrowser] No handler for: " << path << std::endl;
                }
            }
        }

        // Context menu
        if (ImGui::BeginPopupContextItem()) {
            RenderContextMenu(path, entry.is_directory());
            ImGui::EndPopup();
        }

        // Filename and info
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + m_ThumbnailSize);
        ImGui::TextWrapped("%s", filename.c_str());

        // File size (only for files)
        if (!entry.is_directory()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::TextWrapped("%s", GetFileSize(path).c_str());
            ImGui::PopStyleColor();
        }
        ImGui::PopTextWrapPos();

        ImGui::NextColumn();
        ImGui::PopID();
    }

    ImGui::Columns(1);
}

void ContentBrowser::RenderContextMenu(const fs::path& path, bool isDirectory) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // Special actions for specific file types
    if (!isDirectory) {
        if (ext == ".flagscene") {
            if (ImGui::MenuItem("Load Scene")) {
                LoadScene(path);
            }
            ImGui::Separator();
        }
        else if (ext == ".flagobj") {
            if (ImGui::MenuItem("Instantiate Prefab")) {
                LoadPrefab(path);
            }
            ImGui::Separator();
        }
        else if (ext == ".fbx" || ext == ".obj" || ext == ".gltf" || ext == ".glb") {
            if (ImGui::MenuItem("Import Model")) {
                ImportModel(path);
            }
            ImGui::Separator();
        }
        else if (ext == ".flagmat") {
            if (ImGui::MenuItem("Apply to Selected Entity")) {
                // TODO: Apply material to selected entity
                std::cout << "[ContentBrowser] Apply material: " << path << std::endl;
            }
            ImGui::Separator();
        }
    }

    if (ImGui::MenuItem("Copy")) {
        CopyPath(path);
    }

    if (ImGui::MenuItem("Cut")) {
        CutPath(path);
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Rename")) {
        m_RenamingPath = path;
        strncpy_s(m_RenameBuffer, path.filename().string().c_str(), sizeof(m_RenameBuffer));
    }

    if (ImGui::MenuItem("Delete")) {
        DeletePath(path);
        if (m_SelectedPath == path) {
            m_SelectedPath.clear();
        }
    }

    if (!isDirectory) {
        ImGui::Separator();
        if (ImGui::MenuItem("Open in External Editor")) {
            OpenFileInExternalEditor(path);
        }
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Show in Explorer")) {
#ifdef _WIN32
        std::string cmd = "explorer /select,\"" + path.string() + "\"";
        system(cmd.c_str());
#endif
    }

    // File info
    ImGui::Separator();
    ImGui::TextDisabled("Path: %s", GetRelativePath(path).c_str());
    if (!isDirectory) {
        ImGui::TextDisabled("Size: %s", GetFileSize(path).c_str());
    }
}

void ContentBrowser::RenderEmptySpaceContextMenu() {
    if (ImGui::BeginPopupContextWindow()) {
        ImGui::SeparatorText("Create");

        if (ImGui::MenuItem("New Scene")) {
            strcpy_s(m_NewFileNameBuffer, "NewScene");
            ImGui::OpenPopup("CreateNewScene");
        }

        if (ImGui::MenuItem("New Material")) {
            strcpy_s(m_NewFileNameBuffer, "NewMaterial");
            ImGui::OpenPopup("CreateNewMaterial");
        }

        if (ImGui::MenuItem("New Folder")) {
            CreateNewFolder("NewFolder");
        }

        ImGui::Separator();

        if (!m_CopiedPath.empty() && ImGui::MenuItem("Paste")) {
            PasteToCurrentDirectory();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Refresh")) {
            RefreshCurrentDirectory();
        }

        ImGui::Separator();
        ImGui::MenuItem("Show Hidden Files", nullptr, &m_ShowHiddenFiles);

        ImGui::EndPopup();
    }
}

std::string ContentBrowser::GetRelativePath(const fs::path& path) {
    try {
        return fs::relative(path, m_ProjectRoot).generic_string();
    }
    catch (...) {
        return path.string();
    }
}

// ============================================================================
// ASSET OPERATIONS
// ============================================================================

void ContentBrowser::LoadScene(const fs::path& scenePath) {
    if (!m_ActiveScene || !m_PBRShader) {
        std::cout << "[ContentBrowser] Cannot load scene: Scene or Shader not set!" << std::endl;
        return;
    }

    std::cout << "[ContentBrowser] Loading scene: " << scenePath << std::endl;
    ProjectManager::LoadScene(*m_ActiveScene, scenePath.string(), m_PBRShader);
}

void ContentBrowser::CreateNewScene(const std::string& name) {
    if (!m_ActiveScene) {
        std::cout << "[ContentBrowser] Cannot create scene: No active scene!" << std::endl;
        return;
    }

    // Clear current scene
    m_ActiveScene->entities.clear();
    m_ActiveScene->world.entities.clear();
    m_ActiveScene->world.names.clear();
    m_ActiveScene->world.transforms.clear();
    m_ActiveScene->world.renders.clear();
    m_ActiveScene->world.bounds.clear();
    m_ActiveScene->world.lights.clear();
    m_ActiveScene->world.models.clear();
    m_ActiveScene->world.materials.clear();
    m_ActiveScene->world.nextId = 0;
    m_ActiveScene->selectedEntity = { UINT32_MAX };
    m_ActiveScene->selectedIndex = -1;

    // Save to current directory or Scenes folder
    fs::path scenesFolder = m_ProjectRoot / "Scenes";
    if (!fs::exists(scenesFolder)) {
        fs::create_directories(scenesFolder);
    }

    std::string scenePath = (scenesFolder / (name + ".flagSCENE")).string();
    ProjectManager::SaveActiveScene(*m_ActiveScene, name);

    std::cout << "[ContentBrowser] Created new scene: " << name << std::endl;
}

void ContentBrowser::SaveCurrentSceneAs(const std::string& name) {
    if (!m_ActiveScene) {
        std::cout << "[ContentBrowser] Cannot save scene: No active scene!" << std::endl;
        return;
    }

    ProjectManager::SaveActiveScene(*m_ActiveScene, name);
    std::cout << "[ContentBrowser] Saved scene as: " << name << std::endl;
}

void ContentBrowser::CreatePrefabFromSelection(const std::string& name) {
    if (!m_ActiveScene) {
        std::cout << "[ContentBrowser] Cannot create prefab: No active scene!" << std::endl;
        return;
    }

    Entity* selected = m_ActiveScene->Selected();
    if (!selected) {
        std::cout << "[ContentBrowser] Cannot create prefab: No entity selected!" << std::endl;
        return;
    }

    // Save to Prefabs folder
    fs::path prefabsFolder = m_ProjectRoot / "Assets" / "Prefabs";
    if (!fs::exists(prefabsFolder)) {
        fs::create_directories(prefabsFolder);
    }

    ProjectManager::SaveEntityAsPrefab(*m_ActiveScene, *selected, name);
    std::cout << "[ContentBrowser] Created prefab: " << name << std::endl;
}

void ContentBrowser::LoadPrefab(const fs::path& prefabPath) {
    if (!m_ActiveScene || !m_PBRShader) {
        std::cout << "[ContentBrowser] Cannot load prefab: Scene or Shader not set!" << std::endl;
        return;
    }

    std::string relativePath = GetRelativePath(prefabPath);
    std::cout << "[ContentBrowser] Loading prefab: " << prefabPath << std::endl;
    ProjectManager::LoadPrefab(*m_ActiveScene, relativePath, m_PBRShader);
}

void ContentBrowser::CreateMaterial(const std::string& name) {
    if (!m_PBRShader) {
        std::cout << "[ContentBrowser] Cannot create material: No PBR shader!" << std::endl;
        return;
    }

    // Create default material
    Material mat;
    mat.shader = m_PBRShader;
    mat.baseColor = glm::vec3(0.8f);
    mat.metallic = 0.0f;
    mat.roughness = 0.5f;
    mat.ao = 1.0f;

    ProjectManager::SaveMaterial(mat, name);
    std::cout << "[ContentBrowser] Created material: " << name << std::endl;
}

void ContentBrowser::ImportModel(const fs::path& modelPath) {
    if (!m_ActiveScene) {
        std::cout << "[ContentBrowser] Cannot import model: No active scene!" << std::endl;
        return;
    }

    std::cout << "[ContentBrowser] Importing model: " << modelPath << std::endl;
    Importer::ImportModel(*m_ActiveScene, modelPath.string());
}

void ContentBrowser::OpenFileInExternalEditor(const fs::path& path) {
#ifdef _WIN32
    std::string cmd = "start \"\" \"" + path.string() + "\"";
    system(cmd.c_str());
#else
    std::string cmd = "xdg-open \"" + path.string() + "\"";
    system(cmd.c_str());
#endif
}

void ContentBrowser::RenderQuickActions() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 6));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.2f, 1.0f));

    if (ImGui::Button("+ Create")) {
        m_ShowCreateMenu = true;
        ImGui::OpenPopup("CreateAsset");
    }

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.3f, 0.2f, 1.0f));
    if (ImGui::Button("Save Scene As...")) {
        ImGui::OpenPopup("SaveSceneAs");
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.3f, 0.5f, 1.0f));
    bool hasSelection = m_ActiveScene && m_ActiveScene->Selected();
    if (!hasSelection) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    }
    if (ImGui::Button("Create Prefab from Selection")) {
        ImGui::OpenPopup("CreatePrefab");
    }
    if (!hasSelection) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
    ImGui::PopStyleColor();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void ContentBrowser::RenderCreateMenu() {
    if (ImGui::BeginPopup("CreateAsset")) {
        ImGui::SeparatorText("Create New Asset");

        if (ImGui::MenuItem("New Scene")) {
            strcpy_s(m_NewFileNameBuffer, "NewScene");
            ImGui::OpenPopup("CreateNewScene");
        }

        if (ImGui::MenuItem("New Material")) {
            strcpy_s(m_NewFileNameBuffer, "NewMaterial");
            ImGui::OpenPopup("CreateNewMaterial");
        }

        ImGui::Separator();

        if (ImGui::MenuItem("New Folder")) {
            CreateNewFolder("NewFolder");
        }

        ImGui::EndPopup();
    }

    // Create New Scene Dialog
    if (ImGui::BeginPopupModal("CreateNewScene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter scene name:");
        ImGui::InputText("##SceneName", m_NewFileNameBuffer, sizeof(m_NewFileNameBuffer));

        if (ImGui::Button("Create", ImVec2(120, 0))) {
            CreateNewScene(m_NewFileNameBuffer);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Create New Material Dialog
    if (ImGui::BeginPopupModal("CreateNewMaterial", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter material name:");
        ImGui::InputText("##MaterialName", m_NewFileNameBuffer, sizeof(m_NewFileNameBuffer));

        if (ImGui::Button("Create", ImVec2(120, 0))) {
            CreateMaterial(m_NewFileNameBuffer);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Save Scene As Dialog
    if (ImGui::BeginPopupModal("SaveSceneAs", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter scene name:");
        ImGui::InputText("##SaveAsName", m_NewFileNameBuffer, sizeof(m_NewFileNameBuffer));

        if (ImGui::Button("Save", ImVec2(120, 0))) {
            SaveCurrentSceneAs(m_NewFileNameBuffer);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Create Prefab Dialog
    if (ImGui::BeginPopupModal("CreatePrefab", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (m_ActiveScene && m_ActiveScene->Selected()) {
            std::string entityName = m_ActiveScene->world.getName(*m_ActiveScene->Selected()).name;
            ImGui::Text("Create prefab from: %s", entityName.c_str());
        }
        ImGui::Text("Enter prefab name:");
        ImGui::InputText("##PrefabName", m_NewFileNameBuffer, sizeof(m_NewFileNameBuffer));

        if (ImGui::Button("Create", ImVec2(120, 0))) {
            CreatePrefabFromSelection(m_NewFileNameBuffer);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void ContentBrowser::OnImGuiRender() {
    // Update project root if project is loaded
    if (ProjectManager::IsProjectLoaded && m_ProjectRoot.empty()) {
        SetProjectRoot(ProjectManager::CurrentProject.fullProjectPath);
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.14f, 1.00f));
    ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_NoCollapse);

    if (!ProjectManager::IsProjectLoaded) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);
        ImGui::TextDisabled("No project loaded.");
        ImGui::TextDisabled("Create or open a project to browse files.");
        ImGui::End();
        ImGui::PopStyleColor();
        return;
    }

    // Breadcrumbs
    RenderBreadcrumbs();

    ImGui::Separator();

    // Quick Actions Toolbar
    RenderQuickActions();

    ImGui::Separator();

    // Toolbar
    RenderToolbar();

    ImGui::Separator();

    // File grid
    RenderFileGrid();

    // Empty space context menu
    RenderEmptySpaceContextMenu();

    // Create menus
    RenderCreateMenu();

    // Rename dialog
    if (!m_RenamingPath.empty()) {
        ImGui::OpenPopup("Rename");
        if (ImGui::BeginPopupModal("Rename", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("New name:");
            ImGui::InputText("##NewName", m_RenameBuffer, sizeof(m_RenameBuffer));

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                RenamePath(m_RenamingPath, m_RenameBuffer);
                m_RenamingPath.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                m_RenamingPath.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    ImGui::End();
    ImGui::PopStyleColor();
}