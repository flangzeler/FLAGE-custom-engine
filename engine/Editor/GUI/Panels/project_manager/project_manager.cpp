#include <glad/glad.h>
#include <windows.h>
#undef APIENTRY
#include "project_manager.h"
#include "imgui.h"
#include <fstream>
#include <iostream>
#include <commdlg.h>
#include <filesystem>

// Engine Core Importer
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\importer\importer.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\shaders\PBR_source\PBR_shader.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\shaders\shaders_default.h"


namespace fs = std::filesystem;
using json = nlohmann::json;

// --- GLOBAL PBR SHADER (initialized once, matches inspector pattern) ---
static PBR_shader* g_PBRShaderSource = nullptr;
static Shader* g_GlobalPBRShader = nullptr;

void ProjectManager::LogInfo(const std::string& msg) {
    std::cout << "[FLAGE INFO] " << msg << std::endl;
}

static Shader* GetGlobalPBRShader() {
    if (g_GlobalPBRShader == nullptr) {
      ProjectManager:: LogInfo("Initializing global PBR shader...");
        g_PBRShaderSource = new PBR_shader();
        g_GlobalPBRShader = new Shader(
            g_PBRShaderSource->vertexSrc,
            g_PBRShaderSource->fragmentSrc,
            ShaderType::PBR
        );
       ProjectManager:: LogInfo("Global PBR shader created - ID: " + std::to_string(g_GlobalPBRShader->ID));
    }
    return g_GlobalPBRShader;
}

// --- Static Member Initialization ---
bool ProjectManager::IsProjectLoaded = false;
ProjectSettings ProjectManager::CurrentProject = {};

// --- Internal UI State ---
static char UI_ProjName[128] = "MyNewProject";
static char UI_ProjPath[256] = "C:/Projects";

// --- LOGGING SYSTEM ---
void ProjectManager::LogError(const std::string& msg) {
    std::cerr << "[FLAGE ERROR] " << msg << std::endl;
}

void ProjectManager::LogWarning(const std::string& msg) {
    std::cout << "[FLAGE WARNING] " << msg << std::endl;
}



// --- NATIVE WINDOWS LOADER ---
std::string OpenNativeProjDialog() {
    OPENFILENAMEA ofn;
    char szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "FLAGE Project (*.flagPROJ)\0*.flagPROJ\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return std::string(ofn.lpstrFile);
    }
    return "";
}

// --- PATH UTILITIES ---
std::string ProjectManager::ToRelative(const std::string& absolutePath) {
    if (!IsProjectLoaded || absolutePath.empty()) return absolutePath;
    try {
        fs::path abs(absolutePath);
        fs::path root(CurrentProject.fullProjectPath);
        if (root.empty()) return absolutePath;
        return fs::relative(abs, root).generic_string();
    }
    catch (const std::exception& e) {
        LogWarning("Path conversion failed: " + std::string(e.what()));
        return absolutePath;
    }
}

std::string ProjectManager::ToAbsolute(const std::string& relativePath) {
    if (relativePath.empty()) return "";
    try {
        fs::path rel(relativePath);
        if (rel.is_absolute()) return relativePath;
        if (!IsProjectLoaded || CurrentProject.fullProjectPath.empty()) return "";
        return (fs::path(CurrentProject.fullProjectPath) / rel).generic_string();
    }
    catch (const std::exception& e) {
        LogWarning("Path conversion failed: " + std::string(e.what()));
        return "";
    }
}

// ============================================================================
// COMPONENT SERIALIZATION
// ============================================================================

json ProjectManager::SerializeTransform(const TransformComponent& t) {
    return {
        {"position", {t.position.x, t.position.y, t.position.z}},
        {"rotation", {t.rotation.x, t.rotation.y, t.rotation.z}},
        {"scale", {t.scale.x, t.scale.y, t.scale.z}}
    };
}

TransformComponent ProjectManager::DeSerializeTransform(const json& j) {
    TransformComponent t;
    if (j.contains("position")) {
        t.position = glm::vec3(j["position"][0], j["position"][1], j["position"][2]);
    }
    if (j.contains("rotation")) {
        t.rotation = glm::vec3(j["rotation"][0], j["rotation"][1], j["rotation"][2]);
    }
    if (j.contains("scale")) {
        t.scale = glm::vec3(j["scale"][0], j["scale"][1], j["scale"][2]);
    }
    return t;
}

json ProjectManager::SerializeRender(const RenderComponent& r) {
    return {
        {"shape", (int)r.shape},
        {"visible", r.visible},
        {"useTexture", r.useTexture},
        {"textureID", r.textureID},
        {"ambientStrength", r.ambientStrength},
        {"specularStrength", r.specularStrength},
        {"shininess", r.shininess},
        {"baseColor", {r.baseColor.r, r.baseColor.g, r.baseColor.b}},
        {"selected", r.selected},
        {"texturePath", ToRelative(r.texturePath)}
    };
}

RenderComponent ProjectManager::DeSerializeRender(const json& j) {
    RenderComponent r;
    r.shape = (ShapeType)j.value("shape", 0);
    r.visible = j.value("visible", true);
    r.useTexture = j.value("useTexture", false);
    r.textureID = j.value("textureID", 0);
    r.ambientStrength = j.value("ambientStrength", 0.2f);
    r.specularStrength = j.value("specularStrength", 0.5f);
    r.shininess = j.value("shininess", 32.0f);

    if (j.contains("baseColor")) {
        r.baseColor = glm::vec3(j["baseColor"][0], j["baseColor"][1], j["baseColor"][2]);
    }
    r.selected = j.value("selected", false);
    r.texturePath = ToAbsolute(j.value("texturePath", ""));
    return r;
}

json ProjectManager::SerializeLight(const LightComponent& l) {
    return {
        {"type", (int)l.type},
        {"position", {l.position.x, l.position.y, l.position.z}},
        {"direction", {l.direction.x, l.direction.y, l.direction.z}},
        {"color", {l.color.r, l.color.g, l.color.b}},
        {"intensity", l.intensity},
        {"cutoff", l.cutoff},
        {"outerCutoff", l.outerCutoff}
    };
}

LightComponent ProjectManager::DeSerializeLight(const json& j) {
    LightComponent l;
    l.type = (LightType)j.value("type", 0);

    if (j.contains("position")) {
        l.position = glm::vec3(j["position"][0], j["position"][1], j["position"][2]);
    }
    if (j.contains("direction")) {
        l.direction = glm::vec3(j["direction"][0], j["direction"][1], j["direction"][2]);
    }
    if (j.contains("color")) {
        l.color = glm::vec3(j["color"][0], j["color"][1], j["color"][2]);
    }

    l.intensity = j.value("intensity", 1.0f);
    l.cutoff = j.value("cutoff", glm::cos(glm::radians(12.5f)));
    l.outerCutoff = j.value("outerCutoff", glm::cos(glm::radians(17.5f)));
    return l;
}

json ProjectManager::SerializeModel(const ModelComponent& m) {
    return {
        {"path", ToRelative(m.path)},
        {"VAO", m.VAO},
        {"VBO", m.VBO},
        {"EBO", m.EBO}
        // Note: vertices and indices are regenerated on load from the model file
    };
}

ModelComponent ProjectManager::DeSerializeModel(const json& j) {
    ModelComponent m;
    m.path = ToAbsolute(j.value("path", ""));
    // VAO, VBO, EBO will be regenerated by the importer
    return m;
}

// ============================================================================
// MATERIAL SERIALIZATION
// ============================================================================

json ProjectManager::SerializeMaterial(const Material& mat) {
    json j;
    j["version"] = FLAGE_MATERIAL_VERSION;

    // Base properties
    j["baseColor"] = { mat.baseColor.r, mat.baseColor.g, mat.baseColor.b };
    j["metallic"] = mat.metallic;
    j["roughness"] = mat.roughness;
    j["ao"] = mat.ao;
    j["opacity"] = mat.opacity;
    j["clearCoat"] = mat.clearCoat;
    j["clearCoatRoughness"] = mat.clearCoatRoughness;
    j["emissive"] = mat.Emissive;

    // Texture paths (relative to project)
    j["textures"] = {
        {"albedo", ToRelative(mat.albedoPath)},
        {"normal", ToRelative(mat.normalPath)},
        {"metallic", ToRelative(mat.metallicPath)},
        {"roughness", ToRelative(mat.roughnessPath)},
        {"ao", ToRelative(mat.aoPath)},
        {"opacity", ToRelative(mat.opacityPath)},
        {"mrao", ToRelative(mat.mraoPath)},
        {"emissive", ToRelative(mat.emissivePath)},
        {"clearCoat", ToRelative(mat.clearCoatPath)},
        {"clearCoatRoughness", ToRelative(mat.clearCoatRoughnessPath)}
    };

    // Feature flags
    j["flags"] = {
        {"useAlbedo", mat.useAlbedo},
        {"useNormal", mat.useNormal},
        {"useMetallicMap", mat.useMetallicMap},
        {"useRoughnessMap", mat.useRoughnessMap},
        {"useAOMap", mat.useAOMap},
        {"useOpacityMap", mat.useOpacityMap},
        {"useClearCoat", mat.useClearCoat},
        {"useMRAO", mat.useMRAO},
        {"useEmissive", mat.useEmissive},
        {"useIBL", mat.useIBL},
        {"useFog", mat.useFog}
    };

    // Fog settings
    j["fog"] = {
        {"color", {mat.fogColor.r, mat.fogColor.g, mat.fogColor.b}},
        {"density", mat.fogDensity}
    };

    return j;
}

bool ProjectManager::DeSerializeMaterial(Material& mat, const json& j) {
    try {
        // Check version (for future compatibility)
        std::string version = j.value("version", "1.0");
        LogInfo("  [MAT] DeSerializing material (version: " + version + ")");

        // Base properties
        if (j.contains("baseColor")) {
            mat.baseColor = glm::vec3(j["baseColor"][0], j["baseColor"][1], j["baseColor"][2]);
        }
        mat.metallic = j.value("metallic", 0.0f);
        mat.roughness = j.value("roughness", 0.5f);
        mat.ao = j.value("ao", 1.0f);
        mat.opacity = j.value("opacity", 1.0f);
        mat.clearCoat = j.value("clearCoat", 0.0f);
        mat.clearCoatRoughness = j.value("clearCoatRoughness", 0.03f);
        mat.Emissive = j.value("emissive", 0.0f);

        LogInfo("  [MAT] Base Color: (" + std::to_string(mat.baseColor.r) + "," +
            std::to_string(mat.baseColor.g) + "," + std::to_string(mat.baseColor.b) + ")");
        LogInfo("  [MAT] Metallic: " + std::to_string(mat.metallic) + " Roughness: " + std::to_string(mat.roughness));

        // Texture paths
        if (j.contains("textures")) {
            auto& tex = j["textures"];
            mat.albedoPath = ToAbsolute(tex.value("albedo", ""));
            mat.normalPath = ToAbsolute(tex.value("normal", ""));
            mat.metallicPath = ToAbsolute(tex.value("metallic", ""));
            mat.roughnessPath = ToAbsolute(tex.value("roughness", ""));
            mat.aoPath = ToAbsolute(tex.value("ao", ""));
            mat.opacityPath = ToAbsolute(tex.value("opacity", ""));
            mat.mraoPath = ToAbsolute(tex.value("mrao", ""));
            mat.emissivePath = ToAbsolute(tex.value("emissive", ""));
            mat.clearCoatPath = ToAbsolute(tex.value("clearCoat", ""));
            mat.clearCoatRoughnessPath = ToAbsolute(tex.value("clearCoatRoughness", ""));

            if (!mat.albedoPath.empty()) LogInfo("  [MAT] Albedo path: " + mat.albedoPath);
            if (!mat.normalPath.empty()) LogInfo("  [MAT] Normal path: " + mat.normalPath);
            if (!mat.mraoPath.empty()) LogInfo("  [MAT] MRAO path: " + mat.mraoPath);
        }

        // Feature flags
        if (j.contains("flags")) {
            auto& flags = j["flags"];
            mat.useAlbedo = flags.value("useAlbedo", false);
            mat.useNormal = flags.value("useNormal", false);
            mat.useMetallicMap = flags.value("useMetallicMap", false);
            mat.useRoughnessMap = flags.value("useRoughnessMap", false);
            mat.useAOMap = flags.value("useAOMap", false);
            mat.useOpacityMap = flags.value("useOpacityMap", false);
            mat.useClearCoat = flags.value("useClearCoat", false);
            mat.useMRAO = flags.value("useMRAO", false);
            mat.useEmissive = flags.value("useEmissive", false);
            mat.useIBL = flags.value("useIBL", false);
            mat.useFog = flags.value("useFog", false);

            LogInfo("  [MAT] Flags - useAlbedo: " + std::to_string(mat.useAlbedo) +
                " useNormal: " + std::to_string(mat.useNormal) +
                " useMRAO: " + std::to_string(mat.useMRAO));
        }

        // Fog settings
        if (j.contains("fog")) {
            auto& fog = j["fog"];
            if (fog.contains("color")) {
                mat.fogColor = glm::vec3(fog["color"][0], fog["color"][1], fog["color"][2]);
            }
            mat.fogDensity = fog.value("density", 0.02f);
        }

        // Note: Texture handles (albedoTex, normalTex, etc.) will be loaded by your texture loader
        // The shader pointer should be set by the caller

        return true;
    }
    catch (const std::exception& e) {
        LogError("Material deserialization failed: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::SaveMaterial(const Material& mat, const std::string& matName) {
    if (!IsProjectLoaded) {
        LogError("Cannot save material: No project loaded");
        return false;
    }

    try {
        json j = SerializeMaterial(mat);

        std::string dir = CurrentProject.fullProjectPath + "/Library/Materials/";
        if (!fs::exists(dir)) {
            fs::create_directories(dir);
        }

        std::string filepath = dir + matName + ".flagMAT";
        std::ofstream file(filepath);
        if (!file.is_open()) {
            LogError("Failed to open file for writing: " + filepath);
            return false;
        }

        file << j.dump(4);
        file.close();
        LogInfo("Material saved: " + matName);
        return true;
    }
    catch (const std::exception& e) {
        LogError("Material save failed: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::LoadMaterial(Material& mat, const std::string& matRelativePath) {
    std::string fullPath = ToAbsolute(matRelativePath);
    if (fullPath.empty()) {
        LogError("Invalid material path");
        return false;
    }

    if (!fs::exists(fullPath)) {
        LogError("Material file not found: " + fullPath);
        return false;
    }

    try {
        std::ifstream file(fullPath);
        if (!file.is_open()) {
            LogError("Failed to open material file: " + fullPath);
            return false;
        }

        json j;
        file >> j;
        file.close();

        bool success = DeSerializeMaterial(mat, j);
        if (success) {
            LogInfo("Material loaded: " + matRelativePath);
        }
        return success;
    }
    catch (const std::exception& e) {
        LogError("Material load failed: " + std::string(e.what()));
        return false;
    }
}

// ============================================================================
// ENTITY SERIALIZATION
// ============================================================================

json ProjectManager::SerializeEntity(World& world, Entity& e) {
    json ent;

    // Core entity data - CRITICAL: Preserve the exact ID
    ent["id"] = e.id;
    ent["parent"] = e.parent;
    ent["children"] = e.children;

    // Name component
    if (world.hasName(e)) {
        ent["name"] = world.getName(e).name;
    }

    // Transform component
    if (world.hasTransform(e)) {
        ent["components"]["transform"] = SerializeTransform(world.getTransform(e));
    }

    // Render component (for built-in shapes)
    if (world.hasRender(e)) {
        ent["components"]["render"] = SerializeRender(world.getRender(e));
    }

    // Bounding component
    if (world.hasBounding(e)) {
        ent["components"]["bounding"] = {
            {"radius", world.getBounding(e).radius}
        };
    }

    // Light component
    if (world.hasLight(e)) {
        ent["components"]["light"] = SerializeLight(world.getLight(e));
    }

    // Model component (for imported models)
    if (world.hasModel(e)) {
        ent["components"]["model"] = SerializeModel(world.getModel(e));
    }

    // Material component
    if (world.hasMaterial(e)) {
        std::string matName = "Mat_" + std::to_string(e.id);
        std::string matPath = "Library/Materials/" + matName + ".flagMAT";
        ent["components"]["material_ref"] = matPath;

        // Save material to separate file
        SaveMaterial(world.getMaterial(e).material, matName);
    }

    return ent;
}

bool ProjectManager::DeSerializeEntity(Scene& scene, const json& j, Shader* pbrShader, uint32_t forcedId) {
    try {
        World& world = scene.world;

        // Determine entity ID
        uint32_t entityId;
        if (forcedId == UINT32_MAX) {
            // Generate NEW ID (for prefabs or when we want a fresh entity)
            entityId = world.nextId++;
        }
        else {
            // Use specific ID (for scene loading to preserve relationships)
            entityId = forcedId;
            // Update world's nextId to avoid collisions
            if (entityId >= world.nextId) {
                world.nextId = entityId + 1;
            }
        }

        // Create entity with determined ID
        Entity e;
        e.id = entityId;
        e.parent = j.value("parent", -1);
        e.children = j.value("children", std::vector<uint32_t>());

        // Add to world
        world.entities.push_back(e);

        // Deserialize name
        if (j.contains("name")) {
            world.addName(e, j["name"]);
        }

        auto& comp = j["components"];

        // Deserialize transform
        if (comp.contains("transform")) {
            world.addTransform(e, DeSerializeTransform(comp["transform"]));
        }

        // Deserialize render component
        if (comp.contains("render")) {
            world.addRender(e, DeSerializeRender(comp["render"]));
        }

        // Deserialize bounding
        if (comp.contains("bounding")) {
            BoundingComponent b;
            b.radius = comp["bounding"].value("radius", 1.0f);
            world.addBounding(e, b);
        }

        // Deserialize light
        if (comp.contains("light")) {
            world.addLight(e, DeSerializeLight(comp["light"]));
        }

        // Deserialize model (imported meshes)
        if (comp.contains("model")) {
            ModelComponent modelComp = DeSerializeModel(comp["model"]);
            LogInfo("Deserializing model component for entity " + std::to_string(e.id) + ": " + modelComp.path);

            // Re-import the model to regenerate GPU buffers
            if (!modelComp.path.empty() && fs::exists(modelComp.path)) {
                LogInfo("Re-importing model: " + modelComp.path);

                // Create a completely separate temporary scene for importing
                Scene tempImportScene;
                tempImportScene.world.nextId = 999999; // Use high IDs to avoid conflicts

                // Import model into temp scene
                Importer::ImportModel(tempImportScene, modelComp.path);

                // If import succeeded, steal the GPU data
                if (!tempImportScene.entities.empty() && tempImportScene.world.hasModel(tempImportScene.entities[0])) {
                    ModelComponent& importedModel = tempImportScene.world.getModel(tempImportScene.entities[0]);

                    LogInfo("Model imported successfully - VAO: " + std::to_string(importedModel.VAO) +
                        " VBO: " + std::to_string(importedModel.VBO) +
                        " EBO: " + std::to_string(importedModel.EBO));

                    // Create new model component with imported GPU data
                    ModelComponent finalModel;
                    finalModel.path = modelComp.path;
                    finalModel.VAO = importedModel.VAO;
                    finalModel.VBO = importedModel.VBO;
                    finalModel.EBO = importedModel.EBO;
                    finalModel.vertices = std::move(importedModel.vertices);
                    finalModel.indices = std::move(importedModel.indices);

                    world.addModel(e, finalModel);

                    // CRITICAL FIX: Ensure render component knows this is a model
                    if (world.hasRender(e)) {
                        world.getRender(e).shape = ShapeType::Model;
                        world.getRender(e).visible = true;
                    }

                    // Clear GPU handles from temp so they don't get deleted
                    importedModel.VAO = 0;
                    importedModel.VBO = 0;
                    importedModel.EBO = 0;
                }
                else {
                    LogWarning("Failed to import model for entity " + std::to_string(e.id) + ": " + modelComp.path);
                }

                // Temp scene will be destroyed here, cleaning up any leftover data
            }
            else {
                LogWarning("Model file not found or empty path: " + modelComp.path);
            }
        }

        // Deserialize material
        if (comp.contains("material_ref")) {
            LogInfo("Entity " + std::to_string(e.id) + " has material_ref: " + comp["material_ref"].get<std::string>());

            // CRITICAL FIX: Use global PBR shader if none provided
            Shader* activeShader = pbrShader;
            if (activeShader == nullptr || activeShader->ID == 0) {
                LogWarning("  pbrShader is NULL or invalid, using global PBR shader");
                activeShader = GetGlobalPBRShader();
            }

            // Verify shader is valid
            if (activeShader == nullptr) {
                LogError("  [CRITICAL] No valid shader available!");
                return false;
            }
            else if (activeShader->ID == 0) {
                LogError("  [CRITICAL] Shader ID is 0 (not compiled)!");
                return false;
            }
            else {
                LogInfo("  [OK] Using shader - ID: " + std::to_string(activeShader->ID));
            }

            MaterialComponent matComp;
            matComp.material.shader = activeShader;
            world.addMaterial(e, matComp);

            // Load material data from file
            if (LoadMaterial(world.getMaterial(e).material, comp["material_ref"])) {
                // Verify shader is still set
                Material& loadedMat = world.getMaterial(e).material;
                if (loadedMat.shader == nullptr) {
                    LogError("  [CRITICAL] Material shader became NULL after loading!");
                }
                else if (loadedMat.shader->ID == 0) {
                    LogError("  [CRITICAL] Material shader ID became 0 after loading!");
                }
                else {
                    LogInfo("  [OK] Material shader still valid after load - ID: " + std::to_string(loadedMat.shader->ID));
                }

                // CRITICAL FIX: Load actual texture files from the paths
                LogInfo("  Texture paths to load:");
                if (loadedMat.useAlbedo && !loadedMat.albedoPath.empty()) {
                    LogInfo("    Albedo: " + loadedMat.albedoPath + " (exists: " + std::to_string(fs::exists(loadedMat.albedoPath)) + ")");
                }
                if (loadedMat.useNormal && !loadedMat.normalPath.empty()) {
                    LogInfo("    Normal: " + loadedMat.normalPath + " (exists: " + std::to_string(fs::exists(loadedMat.normalPath)) + ")");
                }
                if (loadedMat.useMRAO && !loadedMat.mraoPath.empty()) {
                    LogInfo("    MRAO: " + loadedMat.mraoPath + " (exists: " + std::to_string(fs::exists(loadedMat.mraoPath)) + ")");
                }

                LogInfo("  Calling LoadTexturesIfNeeded()...");
                loadedMat.LoadTexturesIfNeeded();

                // Verify textures loaded
                LogInfo("  [TEXTURES] After loading:");
                LogInfo("    albedoTex: " + std::to_string(loadedMat.albedoTex) + " (use: " + std::to_string(loadedMat.useAlbedo) + ")");
                LogInfo("    normalTex: " + std::to_string(loadedMat.normalTex) + " (use: " + std::to_string(loadedMat.useNormal) + ")");
                LogInfo("    mraoTex: " + std::to_string(loadedMat.mraoTex) + " (use: " + std::to_string(loadedMat.useMRAO) + ")");

                LogInfo("Material fully loaded for entity " + std::to_string(e.id));
            }
            else {
                LogError("Failed to load material for entity " + std::to_string(e.id));
            }
        }

        // Add entity to scene
        scene.entities.push_back(e);

        // Final verification log
        LogInfo("Entity " + std::to_string(e.id) + " added to scene:");
        LogInfo("  hasTransform: " + std::to_string(world.hasTransform(e)));
        LogInfo("  hasRender: " + std::to_string(world.hasRender(e)));
        if (world.hasRender(e)) {
            LogInfo("    shape: " + std::to_string((int)world.getRender(e).shape) + " visible: " + std::to_string(world.getRender(e).visible));
        }
        LogInfo("  hasModel: " + std::to_string(world.hasModel(e)));
        LogInfo("  hasMaterial: " + std::to_string(world.hasMaterial(e)));
        LogInfo("  hasLight: " + std::to_string(world.hasLight(e)));

        return true;
    }
    catch (const std::exception& e) {
        LogError("Entity deserialization failed: " + std::string(e.what()));
        return false;
    }
}

// ============================================================================
// SCENE SERIALIZATION
// ============================================================================

bool ProjectManager::SaveActiveScene(Scene& scene, const std::string& sceneName) {
    if (!IsProjectLoaded) {
        LogError("Cannot save scene: No project loaded");
        return false;
    }

    try {
        json sceneJson;
        sceneJson["version"] = FLAGE_SCENE_VERSION;
        sceneJson["name"] = sceneName;
        sceneJson["entityCount"] = scene.entities.size();
        sceneJson["nextEntityId"] = scene.world.nextId;

        // Serialize all entities
        sceneJson["entities"] = json::array();
        for (auto& e : scene.entities) {
            sceneJson["entities"].push_back(SerializeEntity(scene.world, e));
        }

        // Ensure directory exists
        std::string dir = CurrentProject.fullProjectPath + "/Scenes/";
        if (!fs::exists(dir)) {
            fs::create_directories(dir);
        }

        std::string filepath = dir + sceneName + ".flagSCENE";
        std::ofstream file(filepath);
        if (!file.is_open()) {
            LogError("Failed to create scene file: " + filepath);
            return false;
        }

        file << sceneJson.dump(4);
        file.close();

        LogInfo("Scene saved: " + sceneName + " (" + std::to_string(scene.entities.size()) + " entities)");
        return true;
    }
    catch (const std::exception& e) {
        LogError("Scene save failed: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::LoadScene(Scene& scene, const std::string& fullPath, Shader* pbrShader) {
    if (fullPath.empty() || !fs::exists(fullPath)) {
        LogError("Scene file not found: " + fullPath);
        return false;
    }

    try {
        std::ifstream file(fullPath);
        if (!file.is_open()) {
            LogError("Failed to open scene file: " + fullPath);
            return false;
        }

        json sceneJson;
        file >> sceneJson;
        file.close();

        // Check version
        std::string version = sceneJson.value("version", "1.0");
        LogInfo("Loading scene version: " + version);

        // Clear existing scene
        scene.entities.clear();
        scene.world.entities.clear();
        scene.world.names.clear();
        scene.world.transforms.clear();
        scene.world.renders.clear();
        scene.world.bounds.clear();
        scene.world.lights.clear();
        scene.world.models.clear();
        scene.world.materials.clear();
        scene.world.nextId = sceneJson.value("nextEntityId", 0);

        scene.selectedEntity = { UINT32_MAX };
        scene.selectedIndex = -1;

        // Deserialize all entities
        if (sceneJson.contains("entities")) {
            int successCount = 0;
            for (auto& entJson : sceneJson["entities"]) {
                // Extract ID from JSON and pass it to preserve entity IDs
                uint32_t savedId = entJson.value("id", (uint32_t)0);
                if (savedId == 0) {
                    savedId = scene.world.nextId;
                }
                if (DeSerializeEntity(scene, entJson, pbrShader, savedId)) {
                    successCount++;
                }
            }
            LogInfo("Scene loaded: " + std::to_string(successCount) + "/" +
                std::to_string(sceneJson["entities"].size()) + " entities");
        }

        return true;
    }
    catch (const std::exception& e) {
        LogError("Scene load failed: " + std::string(e.what()));
        return false;
    }
}

// ============================================================================
// PREFAB SYSTEM
// ============================================================================

bool ProjectManager::SaveEntityAsPrefab(Scene& scene, Entity e, const std::string& assetName) {
    if (!IsProjectLoaded) {
        LogError("Cannot save prefab: No project loaded");
        return false;
    }

    try {
        json prefab;
        prefab["version"] = FLAGE_PREFAB_VERSION;
        prefab["name"] = assetName;
        prefab["entity"] = SerializeEntity(scene.world, e);

        std::string dir = CurrentProject.fullProjectPath + "/Assets/Prefabs/";
        if (!fs::exists(dir)) {
            fs::create_directories(dir);
        }

        std::string filepath = dir + assetName + ".flagOBJ";
        std::ofstream file(filepath);
        if (!file.is_open()) {
            LogError("Failed to create prefab file: " + filepath);
            return false;
        }

        file << prefab.dump(4);
        file.close();

        LogInfo("Prefab saved: " + assetName);
        return true;
    }
    catch (const std::exception& e) {
        LogError("Prefab save failed: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::LoadPrefab(Scene& scene, const std::string& prefabRelPath, Shader* pbrShader) {
    std::string fullPath = ToAbsolute(prefabRelPath);
    if (fullPath.empty() || !fs::exists(fullPath)) {
        LogError("Prefab file not found: " + fullPath);
        return false;
    }

    try {
        std::ifstream file(fullPath);
        if (!file.is_open()) {
            LogError("Failed to open prefab file: " + fullPath);
            return false;
        }

        json prefab;
        file >> prefab;
        file.close();

        // Check version
        std::string version = prefab.value("version", "1.0");

        if (prefab.contains("entity")) {
            // Create new entity from prefab (with new ID)
            return DeSerializeEntity(scene, prefab["entity"], pbrShader, UINT32_MAX);
        }

        return false;
    }
    catch (const std::exception& e) {
        LogError("Prefab load failed: " + std::string(e.what()));
        return false;
    }
}

// ============================================================================
// PROJECT LIFECYCLE
// ============================================================================

void ProjectManager::UpdateProjectManifest() {
    if (!IsProjectLoaded) return;

    try {
        json manifest;
        manifest["version"] = "1.0";
        manifest["projectName"] = CurrentProject.projectName;
        manifest["lastModified"] = std::time(nullptr);

        std::string path = CurrentProject.fullProjectPath + "/" + CurrentProject.projectName + ".flagPROJ";
        std::ofstream file(path);
        if (file.is_open()) {
            file << manifest.dump(4);
            file.close();
        }
    }
    catch (const std::exception& e) {
        LogError("Failed to update project manifest: " + std::string(e.what()));
    }
}

void ProjectManager::SaveAll(Scene& scene) {
    if (!IsProjectLoaded) return;
    SaveActiveScene(scene, "AutoSave");
    UpdateProjectManifest();
    LogInfo("Auto-save completed");
}

bool ProjectManager::CreateNewProject(const std::string& name, const std::string& path) {
    try {
        fs::path fullPath = fs::path(path) / name;

        // Create directory structure
        fs::create_directories(fullPath / "Assets" / "Prefabs");
        fs::create_directories(fullPath / "Assets" / "Models");
        fs::create_directories(fullPath / "Assets" / "Textures");
        fs::create_directories(fullPath / "Library" / "Materials");
        fs::create_directories(fullPath / "Scenes");

        CurrentProject.projectName = name;
        CurrentProject.fullProjectPath = fullPath.generic_string();
        IsProjectLoaded = true;

        UpdateProjectManifest();
        LogInfo("Project created: " + name);
        return true;
    }
    catch (const std::exception& e) {
        LogError("Project creation failed: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::LoadProject(const std::string& path) {
    if (!fs::exists(path)) {
        LogError("Project file not found: " + path);
        return false;
    }

    try {
        CurrentProject.fullProjectPath = fs::path(path).parent_path().generic_string();
        CurrentProject.projectName = fs::path(path).stem().string();
        IsProjectLoaded = true;

        LogInfo("Project loaded: " + CurrentProject.projectName);
        return true;
    }
    catch (const std::exception& e) {
        LogError("Project load failed: " + std::string(e.what()));
        return false;
    }
}

// ============================================================================
// PROJECT LAUNCHER UI
// ============================================================================

void ProjectManager::RenderProjectLauncher() {
    if (IsProjectLoaded) return;

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 300));

    if (ImGui::Begin("Project Launcher", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Create New Project");
        ImGui::Separator();
        ImGui::InputText("Project Name", UI_ProjName, 128);
        ImGui::InputText("Root Path", UI_ProjPath, 256);

        if (ImGui::Button("Create Project", ImVec2(-1, 35))) {
            CreateNewProject(UI_ProjName, UI_ProjPath);
        }

        ImGui::Spacing();
        ImGui::Text("Existing Project");
        ImGui::Separator();

        if (ImGui::Button("Open Existing Project (.flagPROJ)", ImVec2(-1, 35))) {
            std::string selected = OpenNativeProjDialog();
            if (!selected.empty()) {
                LoadProject(selected);
            }
        }

        ImGui::Spacing();
        if (ImGui::Button("Exit Engine", ImVec2(-1, 0))) {
            exit(0);
        }
    }
    ImGui::End();
}