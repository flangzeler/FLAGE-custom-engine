#pragma once

#include <filesystem>
#include <stack>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// =======================
// Forward declarations
// =======================
class Scene;
class Shader;
class Entity;

// =======================
// ContentBrowser
// =======================
class ContentBrowser {
public:
    ContentBrowser();
    ~ContentBrowser();

    void OnImGuiRender();
    void SetProjectRoot(const std::string& projectPath);
    void RefreshCurrentDirectory();

    // Scene & Prefab Integration
    void SetActiveScene(Scene* scene) { m_ActiveScene = scene; }
    void SetPBRShader(Shader* shader) { m_PBRShader = shader; }

    // =======================
    // Core Navigation
    // =======================
    fs::path m_ProjectRoot;
    fs::path m_CurrentPath;
    std::stack<fs::path> m_BackStack;
    std::stack<fs::path> m_ForwardStack;

    // =======================
    // Scene / Shader bindings
    // =======================
    Scene* m_ActiveScene = nullptr;
    Shader* m_PBRShader = nullptr;

    // =======================
    // UI State
    // =======================
    char m_SearchBuffer[256]{};
    char m_RenameBuffer[256]{};
    char m_NewFileNameBuffer[256]{};

    float m_ThumbnailSize = 80.0f;
    bool m_ShowHiddenFiles = false;
    bool m_ShowCreateMenu = false;

    // =======================
    // File Operations
    // =======================
    fs::path m_CopiedPath;
    bool m_IsCutOperation = false;
    fs::path m_SelectedPath;
    fs::path m_RenamingPath;

    // =======================
    // Icons
    // =======================
    unsigned int m_IconFolder = 0;
    unsigned int m_IconFile = 0;
    unsigned int m_IconModel = 0;
    unsigned int m_IconCode = 0;
    unsigned int m_IconImage = 0;
    unsigned int m_IconScene = 0;
    unsigned int m_IconMaterial = 0;
    unsigned int m_IconPrefab = 0;

    // =======================
    // Filtering
    // =======================
    enum class FileFilter {
        All,
        Models,
        Images,
        Scenes,
        Scripts,
        Materials
    };
    FileFilter m_CurrentFilter = FileFilter::All;

    // =======================
    // Helper Methods
    // =======================
    void ChangeDirectory(const fs::path& newPath);
    void NavigateUp();
    void RenderBreadcrumbs();
    void RenderToolbar();
    void RenderQuickActions();
    void RenderFileGrid();
    void RenderContextMenu(const fs::path& path, bool isDirectory);
    void RenderEmptySpaceContextMenu();
    void RenderCreateMenu();

    // =======================
    // File Operations
    // =======================
    void DeletePath(const fs::path& path);
    void CopyPath(const fs::path& path);
    void CutPath(const fs::path& path);
    void PasteToCurrentDirectory();
    void RenamePath(const fs::path& oldPath, const std::string& newName);
    void CreateNewFolder(const std::string& name);

    // =======================
    // Asset Operations
    // =======================
    void LoadScene(const fs::path& scenePath);
    void CreateNewScene(const std::string& name);
    void SaveCurrentSceneAs(const std::string& name);
    void CreatePrefabFromSelection(const std::string& name);
    void LoadPrefab(const fs::path& prefabPath);
    void CreateMaterial(const std::string& name);
    void ImportModel(const fs::path& modelPath);
    void OpenFileInExternalEditor(const fs::path& path);

    // =======================
    // Utility
    // =======================
    unsigned int GetIconForPath(const fs::path& path);
    unsigned int LoadTexture(const char* path);
    bool PassesFilter(const fs::path& path);
    std::string GetFileSize(const fs::path& path);
    bool IsProjectFile(const std::string& extension);
    std::string GetRelativePath(const fs::path& path);
};
