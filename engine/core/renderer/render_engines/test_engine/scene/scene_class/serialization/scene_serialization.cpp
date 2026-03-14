#include "scene_serialization.h"
#include <fstream>
#include <json.hpp>
#include <unordered_map>
#include <string>

#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/acpects/texture_loader/texture_loader.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/importer/importer.h"

// Assimp headers (used here for direct import on load)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// OpenGL
#include <GLFW/glfw3.h>

using json = nlohmann::json;

// Upload helper: matches vertex layout pos(3), normal(3), uv(2)
static void UploadMeshToGL(ModelComponent& mc) {
    glGenVertexArrays(1, &mc.VAO);
    glGenBuffers(1, &mc.VBO);
    glGenBuffers(1, &mc.EBO);

    glBindVertexArray(mc.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mc.VBO);
    glBufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(mc.vertices.size() * sizeof(float)),
        mc.vertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mc.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(mc.indices.size() * sizeof(unsigned int)),
        mc.indices.data(),
        GL_STATIC_DRAW);

    GLsizei stride = 8 * sizeof(float);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // UV
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

// Helper: find entity by id in scene.entities
static Entity* FindSceneEntityById(Scene& scene, uint32_t id) {
    for (auto& e : scene.entities) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

void SceneSerializer::Save(Scene& scene, const std::string& path) {
    json j;
    j["entities"] = json::array();

    for (auto& e : scene.entities) {
        json entityJson;
        entityJson["id"] = e.id;
        entityJson["name"] = scene.world.getName(e).name;

        // Hierarchy
        entityJson["parent"] = e.parent;            // -1 for root
        entityJson["children"] = e.children;        // vector<uint32_t>

        // Transform
        if (scene.world.transforms.count(e.id)) {
            auto& t = scene.world.getTransform(e);
            entityJson["transform"] = {
                {"position", {t.position.x, t.position.y, t.position.z}},
                {"rotation", {t.rotation.x, t.rotation.y, t.rotation.z}},
                {"scale",    {t.scale.x, t.scale.y, t.scale.z}}
            };
        }

        // Render
        if (scene.hasRender(e)) {
            auto& r = scene.world.getRender(e);
            entityJson["render"] = {
                {"shape", (int)r.shape},
                {"visible", r.visible},
                {"useTexture", r.useTexture},
                {"ambientStrength", r.ambientStrength},
                {"specularStrength", r.specularStrength},
                {"shininess", r.shininess},
                {"texturePath", r.useTexture ? r.texturePath : ""}
            };
        }

        // Light (save full data)
        if (scene.hasLight(e)) {
            auto& l = scene.world.getLight(e);
            entityJson["light"] = {
                {"type", (int)l.type},
                {"position", {l.position.x, l.position.y, l.position.z}},
                {"direction", {l.direction.x, l.direction.y, l.direction.z}},
                {"color", {l.color.x, l.color.y, l.color.z}},
                {"intensity", l.intensity},
                {"cutoff", l.cutoff},
                {"outerCutoff", l.outerCutoff}
            };
        }

        // Bounding
        if (scene.world.bounds.count(e.id)) {
            auto& b = scene.world.getBounding(e);
            entityJson["bounding"] = { {"radius", b.radius} };
        }

        // Model (store path only)
        if (scene.world.models.count(e.id)) {
            auto& mc = scene.world.getModel(e);
            entityJson["model"] = { {"path", mc.path} };
        }

        j["entities"].push_back(entityJson);
    }

    std::ofstream file(path);
    file << j.dump(4);
}

void SceneSerializer::Load(Scene& scene, const std::string& path) {
    std::ifstream file(path);
    json j;
    file >> j;

    // Clear old scene
    scene.entities.clear();
    scene.world.entities.clear();
    scene.world.names.clear();
    scene.world.transforms.clear();
    scene.world.renders.clear();
    scene.world.lights.clear();
    scene.world.bounds.clear();
    scene.world.models.clear();

    // Maps for id remapping and parent reconstruction
    std::unordered_map<uint32_t, uint32_t> oldToNewId;
    std::unordered_map<uint32_t, int> oldParentMap;

    // First pass: create entities and components
    for (auto& entityJson : j["entities"]) {
        Entity e = scene.world.createEntity();
        scene.entities.push_back(e);

        uint32_t newId = scene.entities.back().id;
        uint32_t oldId = entityJson["id"];
        oldToNewId[oldId] = newId;

        // Name
        scene.world.addName(scene.entities.back(), entityJson["name"]);

        // Transform
        if (entityJson.contains("transform")) {
            TransformComponent t;
            auto pos = entityJson["transform"]["position"];
            auto rot = entityJson["transform"]["rotation"];
            auto sca = entityJson["transform"]["scale"];
            t.position = { pos[0], pos[1], pos[2] };
            t.rotation = { rot[0], rot[1], rot[2] };
            t.scale = { sca[0], sca[1], sca[2] };
            scene.world.addTransform(scene.entities.back(), t);
        }

        // Render
        if (entityJson.contains("render")) {
            RenderComponent r;
            r.shape = (ShapeType)entityJson["render"]["shape"].get<int>();
            r.visible = entityJson["render"]["visible"];
            r.useTexture = entityJson["render"]["useTexture"];
            r.ambientStrength = entityJson["render"]["ambientStrength"];
            r.specularStrength = entityJson["render"]["specularStrength"];
            r.shininess = entityJson["render"]["shininess"];

            std::string texPath = entityJson["render"]["texturePath"];
            if (r.useTexture && !texPath.empty()) {
                r.textureID = loadTexture(texPath.c_str());
                r.texturePath = texPath;
            }

            scene.world.addRender(scene.entities.back(), r);
        }

        // Light (load full data)
        if (entityJson.contains("light")) {
            LightComponent l;
            l.type = (LightType)entityJson["light"]["type"].get<int>();

            auto pos = entityJson["light"]["position"];
            auto dir = entityJson["light"]["direction"];
            auto col = entityJson["light"]["color"];

            l.position = { pos[0], pos[1], pos[2] };
            l.direction = { dir[0], dir[1], dir[2] };
            l.color = { col[0], col[1], col[2] };
            l.intensity = entityJson["light"]["intensity"];
            l.cutoff = entityJson["light"].value("cutoff", glm::cos(glm::radians(12.5f)));
            l.outerCutoff = entityJson["light"].value("outerCutoff", glm::cos(glm::radians(17.5f)));

            scene.world.addLight(scene.entities.back(), l);
        }

        // Bounding
        if (entityJson.contains("bounding")) {
            BoundingComponent b;
            b.radius = entityJson["bounding"]["radius"];
            scene.world.addBounding(scene.entities.back(), b);
        }

        // Model: import and attach to this entity
        if (entityJson.contains("model")) {
            std::string modelPath = entityJson["model"]["path"];

            Assimp::Importer importer;
            const aiScene* aiSceneObj = importer.ReadFile(
                modelPath,
                aiProcess_Triangulate | aiProcess_CalcTangentSpace
            );

            if (aiSceneObj && !(aiSceneObj->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && aiSceneObj->mRootNode) {
                ModelComponent mc;
                mc.path = modelPath;

                unsigned int vertexOffset = 0;
                for (unsigned int m = 0; m < aiSceneObj->mNumMeshes; ++m) {
                    aiMesh* mesh = aiSceneObj->mMeshes[m];

                    // Vertices: pos(3), normal(3), uv(2)
                    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                        // Position
                        mc.vertices.push_back(mesh->mVertices[i].x);
                        mc.vertices.push_back(mesh->mVertices[i].y);
                        mc.vertices.push_back(mesh->mVertices[i].z);

                        // Normal
                        if (mesh->HasNormals()) {
                            mc.vertices.push_back(mesh->mNormals[i].x);
                            mc.vertices.push_back(mesh->mNormals[i].y);
                            mc.vertices.push_back(mesh->mNormals[i].z);
                        }
                        else {
                            mc.vertices.insert(mc.vertices.end(), { 0.0f, 0.0f, 0.0f });
                        }

                        // UV
                        if (mesh->mTextureCoords[0]) {
                            mc.vertices.push_back(mesh->mTextureCoords[0][i].x);
                            mc.vertices.push_back(mesh->mTextureCoords[0][i].y);
                        }
                        else {
                            mc.vertices.insert(mc.vertices.end(), { 0.0f, 0.0f });
                        }
                    }

                    // Indices (offset by vertex count so far)
                    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                        aiFace face = mesh->mFaces[i];
                        for (unsigned int j = 0; j < face.mNumIndices; j++) {
                            mc.indices.push_back(face.mIndices[j] + vertexOffset);
                        }
                    }

                    vertexOffset += mesh->mNumVertices;
                }

                // Upload buffers and attach
                UploadMeshToGL(mc);
                scene.world.addModel(scene.entities.back(), mc);
            }
        }

        // Store old parent for second pass
        oldParentMap[oldId] = entityJson.value("parent", -1);

        // Initialize hierarchy fields
        Entity* stored = &scene.entities.back();
        stored->parent = -1;
        stored->children.clear();
    }

    // Second pass: rebuild hierarchy
    for (auto& e : scene.entities) {
        // Find old id for this new id
        uint32_t oldIdForThis = 0;
        for (auto& pair : oldToNewId) {
            if (pair.second == e.id) { oldIdForThis = pair.first; break; }
        }

        int oldParentId = oldParentMap[oldIdForThis];
        if (oldParentId != -1) {
            auto it = oldToNewId.find((uint32_t)oldParentId);
            if (it != oldToNewId.end()) {
                uint32_t newParentId = it->second;
                Entity* parent = FindSceneEntityById(scene, newParentId);
                if (parent) {
                    e.parent = parent->id;
                    parent->children.push_back(e.id);
                }
                else {
                    e.parent = -1;
                }
            }
            else {
                e.parent = -1;
            }
        }
        else {
            e.parent = -1; // root
        }
    }
}
