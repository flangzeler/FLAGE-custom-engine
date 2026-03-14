#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <json.hpp>
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\scene\scene_class\scene.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\material\PBR_material\Material.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

// Serialization version for backward compatibility
#define FLAGE_SCENE_VERSION "1.0"
#define FLAGE_MATERIAL_VERSION "1.0"
#define FLAGE_PREFAB_VERSION "1.0"

struct ProjectSettings {
    std::string projectName;
    std::string rootPath;
    std::string fullProjectPath;
    std::vector<std::string> scenePaths;
};

class ProjectManager {
public:
    static bool IsProjectLoaded;
    static ProjectSettings CurrentProject;

    // --- Core Project Management ---
    static bool CreateNewProject(const std::string& name, const std::string& path);
    static bool LoadProject(const std::string& flagProjPath);
    static void RenderProjectLauncher();
    static void UpdateProjectManifest();
    static void SaveAll(Scene& scene);

    // --- Scene Serialization ---
    static bool SaveActiveScene(Scene& scene, const std::string& sceneName);
    static bool LoadScene(Scene& scene, const std::string& fullPath, Shader* pbrShader);

    // --- Material System ---
    static bool SaveMaterial(const Material& mat, const std::string& matName);
    static bool LoadMaterial(Material& mat, const std::string& matRelativePath);

    // --- Prefab System ---
    static bool SaveEntityAsPrefab(Scene& scene, Entity e, const std::string& assetName);
    static bool LoadPrefab(Scene& scene, const std::string& prefabRelPath, Shader* pbrShader);

    // --- Path Helpers ---
    static std::string ToRelative(const std::string& absolutePath);
    static std::string ToAbsolute(const std::string& relativePath);


    // --- Internal Serialization Helpers ---
    static json SerializeEntity(World& world, Entity& e);
    static bool DeSerializeEntity(Scene& scene, const json& j, Shader* pbrShader, uint32_t forcedId = UINT32_MAX);

    static json SerializeMaterial(const Material& mat);
    static bool DeSerializeMaterial(Material& mat, const json& j);

    static json SerializeTransform(const TransformComponent& t);
    static TransformComponent DeSerializeTransform(const json& j);

    static json SerializeRender(const RenderComponent& r);
    static RenderComponent DeSerializeRender(const json& j);

    static json SerializeLight(const LightComponent& l);
    static LightComponent DeSerializeLight(const json& j);

    static json SerializeModel(const ModelComponent& m);
    static ModelComponent DeSerializeModel(const json& j);

    // --- Logging ---
    static void LogError(const std::string& msg);
    static void LogWarning(const std::string& msg);
    static void LogInfo(const std::string& msg);
};