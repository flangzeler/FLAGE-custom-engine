#include "importer.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\Editor\GUI\Panels\project_manager\project_manager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <GLFW/glfw3.h>

namespace fs = std::filesystem;
static void UploadMeshToGL(ModelComponent& mc) {
    glGenVertexArrays(1, &mc.VAO);
    glGenBuffers(1, &mc.VBO);
    glGenBuffers(1, &mc.EBO);

    glBindVertexArray(mc.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mc.VBO);
    glBufferData(GL_ARRAY_BUFFER, mc.vertices.size() * sizeof(float), mc.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mc.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mc.indices.size() * sizeof(unsigned int), mc.indices.data(), GL_STATIC_DRAW);

    // Total floats per vertex: 3(Pos) + 3(Norm) + 2(UV) + 3(Tangent) = 11
    GLsizei stride = 11 * sizeof(float);

    // Position: Location 0 | Offset 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Normal: Location 3 | Offset 3 floats (Pos)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // UVs: Location 2 | Offset 6 floats (Pos + Normal)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Tangent: Location 4 | Offset 8 floats (Pos + Normal + UV)
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);
}
std::string Importer::EnsureUniqueName(Scene& scene, const std::string& base) {
    std::string root = base.empty() ? "Model" : base;
    std::vector<std::string> existing;
    for (auto& e : scene.entities) {
        if (scene.world.hasName(e)) existing.push_back(scene.world.getName(e).name);
    }
    if (std::find(existing.begin(), existing.end(), root) == existing.end()) return root;
    int n = 2;
    while (true) {
        std::string candidate = root + " (" + std::to_string(n) + ")";
        if (std::find(existing.begin(), existing.end(), candidate) == existing.end()) return candidate;
        ++n;
    }
}

Entity Importer::ImportModel(Scene& scene, const std::string& originalPath) {
    std::string finalPath = originalPath;

    // --- STABILITY ADDITION: Project Localization ---
    if (ProjectManager::IsProjectLoaded) {
        std::string projectRoot = ProjectManager::CurrentProject.fullProjectPath;

        // If the model is outside our project, copy it into /Assets
        if (originalPath.find(projectRoot) == std::string::npos) {
            std::string fileName = fs::path(originalPath).filename().string();
            std::string destPath = projectRoot + "/Assets/" + fileName;

            try {
                if (!fs::exists(destPath)) {
                    fs::copy_file(originalPath, destPath, fs::copy_options::overwrite_existing);
                    std::cout << "[Importer] Asset localized to: " << destPath << std::endl;
                }
                finalPath = destPath;
            }
            catch (std::exception& e) {
                std::cerr << "[Importer] Localization failed: " << e.what() << std::endl;
            }
        }
    }

    Assimp::Importer importer;
    const aiScene* aiSceneObj = importer.ReadFile(
        finalPath,
        aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs
    );

    if (!aiSceneObj || aiSceneObj->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiSceneObj->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return { UINT32_MAX };
    }

    std::string uniqueName = EnsureUniqueName(scene, fs::path(finalPath).stem().string());
    Entity e = scene.world.createEntity();
    scene.world.addName(e, uniqueName);

    TransformComponent t;
    t.scale = glm::vec3(1.0f); // Standardized scale
    scene.world.addTransform(e, t);

    ModelComponent mc;
    mc.path = finalPath; // Stores the local project path

    unsigned int vertexOffset = 0;
    for (unsigned int m = 0; m < aiSceneObj->mNumMeshes; ++m) {
        aiMesh* mesh = aiSceneObj->mMeshes[m];
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            // Pos
            mc.vertices.push_back(mesh->mVertices[i].x);
            mc.vertices.push_back(mesh->mVertices[i].y);
            mc.vertices.push_back(mesh->mVertices[i].z);
            // Normal
            if (mesh->HasNormals()) {
                mc.vertices.push_back(mesh->mNormals[i].x);
                mc.vertices.push_back(mesh->mNormals[i].y);
                mc.vertices.push_back(mesh->mNormals[i].z);
            }
            else { mc.vertices.insert(mc.vertices.end(), { 0, 0, 0 }); }
            // UV
            if (mesh->mTextureCoords[0]) {
                mc.vertices.push_back(mesh->mTextureCoords[0][i].x);
                mc.vertices.push_back(mesh->mTextureCoords[0][i].y);
            }
            else { mc.vertices.insert(mc.vertices.end(), { 0, 0 }); }
            // Tangent
            if (mesh->HasTangentsAndBitangents()) {
                mc.vertices.push_back(mesh->mTangents[i].x);
                mc.vertices.push_back(mesh->mTangents[i].y);
                mc.vertices.push_back(mesh->mTangents[i].z);
            }
            else { mc.vertices.insert(mc.vertices.end(), { 1, 0, 0 }); }
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                mc.indices.push_back(face.mIndices[j] + vertexOffset);
        }
        vertexOffset += mesh->mNumVertices;
    }

    UploadMeshToGL(mc);
    scene.world.addModel(e, mc);

    RenderComponent rc(ShapeType::Model);
    rc.baseColor = glm::vec3(1.0f);
    rc.visible = true;
    scene.world.addRender(e, rc);

    scene.world.addBounding(e, { 1.0f });
    scene.entities.push_back(e);

    return e;
}