#include "command_panel.h"
#include "scene.h"
#include <Windows.h>
#undef APIENTRY
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/acpects/texture_loader/texture_loader.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/scene/object_class/object.h"
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\scene\scene_class\scene.h"
CreatePanel::CreatePanel(Scene* scene) : scene(scene) {}

void CreatePanel::Render() {
    ImGui::Begin("Create");

    const ImVec2 btnSize(100, 30);

    // --- Top buttons to switch mode ---
    static bool showShapes = true;
    static bool showLights = false;

    if (ImGui::Button("Shapes", btnSize)) {
        showShapes = true;
        showLights = false;
        showProperties = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Lights", btnSize)) {
        showShapes = false;
        showLights = true;
        showProperties = false;
    }

    ImGui::Separator();

    // --- Shapes section ---
    if (showShapes) {
        ImGui::Text("Geometry");
        ImGui::Separator();

        const ImVec2 shapeBtnSize(150, 30);

        // Toggle buttons for shapes
        auto toggleShapeButton = [&](const char* label, ShapeType type) {
            if (ImGui::Button(label, shapeBtnSize)) {
                if (currentShape == type && showProperties) {
                    showProperties = false; // toggle off
                }
                else {
                    currentShape = type;
                    showProperties = true;  // toggle on
                }
            }
            };

        toggleShapeButton("Cube", ShapeType::Cube);
        ImGui::SameLine();
        toggleShapeButton("Sphere", ShapeType::Sphere);

        toggleShapeButton("Plane", ShapeType::Plane);
        ImGui::SameLine();
        toggleShapeButton("Pyramid", ShapeType::Pyramid);

        toggleShapeButton("Cylinder", ShapeType::Cylinder);
        ImGui::SameLine();
        toggleShapeButton("Cone", ShapeType::Cone);

        toggleShapeButton("Torus", ShapeType::Torus);
        ImGui::SameLine();
        toggleShapeButton("Capsule", ShapeType::Capsule);

        ImGui::Separator();

        // --- Shape properties ---
        if (showProperties) {
            static char nameBuffer[64] = "NewShape";
            static float pos[3] = { 0.0f, 0.0f, 0.0f };
            static float scale[3] = { 1.0f, 1.0f, 1.0f };
            static float rotation[3] = { 0.0f, 0.0f, 0.0f };
            static float ambientStrength = 0.1f;
            static float specularStrength = 1.5f;
            static float shininess = 32.0f;
            static bool useTexture = false;
            static char texturePath[256] = "image.jpg";

            ImGui::Text("Properties");
            ImGui::Separator();

            ImGui::InputText("Name", nameBuffer, IM_ARRAYSIZE(nameBuffer));
            ImGui::DragFloat3("Position", pos, 0.1f);
            ImGui::DragFloat3("Scale", scale, 0.1f);
            ImGui::DragFloat3("Rotation", rotation, 1.0f);
            ImGui::DragFloat("Ambient Strength", &ambientStrength, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Specular Strength", &specularStrength, 0.1f, 0.0f, 10.0f);
            ImGui::DragFloat("Shininess", &shininess, 1.0f, 1.0f, 2048.0f);
            ImGui::Checkbox("Use Texture", &useTexture);

            if (useTexture) {
                ImGui::InputText("Texture Path", texturePath, IM_ARRAYSIZE(texturePath));
                if (ImGui::Button("Browse...")) {
                    OPENFILENAMEW ofn;
                    wchar_t szFile[260] = { 0 };
                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.lpstrFile = szFile;
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFilter = L"Image Files (*.png;*.jpg;*.jpeg;*.bmp)\0*.png;*.jpg;*.jpeg;*.bmp\0";
                    ofn.nFilterIndex = 1;
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                    if (GetOpenFileNameW(&ofn) == TRUE) {
                        char temp[260];
                        size_t converted = 0;
                        wcstombs_s(&converted, temp, sizeof(temp), szFile, _TRUNCATE);
                        strncpy_s(texturePath, sizeof(texturePath), temp, _TRUNCATE);
                    }
                }
            }

            if (ImGui::Button("Confirm", ImVec2(100, 30))) {
                std::string shapeName(nameBuffer);
  
                Entity e = scene->AddEntity(shapeName, currentShape);  
                auto& render = scene->world.getRender(e);
                auto& transform = scene->world.getTransform(e);
                transform.position = { pos[0], pos[1], pos[2] };
                transform.scale = { scale[0], scale[1], scale[2] };
                transform.rotation = { rotation[0], rotation[1], rotation[2] };

              
                render.ambientStrength = ambientStrength;
                render.specularStrength = specularStrength;
                render.shininess = shininess;
                render.useTexture = useTexture;
                render.visible = true;

                if (useTexture) {
                    GLuint texID = loadTexture(texturePath);
                    render.textureID = texID;
                    render.texturePath = texturePath;
                }

                showProperties = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100, 30))) {
                showProperties = false;
            }
        }
    }

    // --- Lights section ---
  // --- Lights section ---
    if (showLights) {
        ImGui::Text("Lights");
        ImGui::Separator();

        const ImVec2 lightBtnSize(150, 30);

        static LightType currentLightType = LightType::Point;
        static bool showLightProperties = false;

        // Toggle buttons for lights
        auto toggleLightButton = [&](const char* label, LightType type, bool sameLine) {
            if (sameLine) ImGui::SameLine();
            if (ImGui::Button(label, lightBtnSize)) {
                if (currentLightType == type && showLightProperties) {
                    showLightProperties = false;
                }
                else {
                    currentLightType = type;
                    showLightProperties = true;
                }
            }
            };

        // Two buttons side by side
        toggleLightButton("Point Light", LightType::Point, false);
        toggleLightButton("Directional Light", LightType::Directional, true);
        // Sun Light button below (Directional, not Spot!)
        toggleLightButton("Sun Light", LightType::Directional, false);

        // Properties only shown when a button is active
        if (showLightProperties) {
            static char lightName[64] = "NewLight";
            static float pos[3] = { 0.0f, 2.0f, 0.0f };
            static float dir[3] = { 0.0f, -1.0f, 0.0f };
            static float col[3] = { 1.0f, 1.0f, 1.0f };
            static float intensity = 1.0f;
            static float cutoff = glm::cos(glm::radians(12.5f));
            static float outerCutoff = glm::cos(glm::radians(17.5f));

            ImGui::Separator();
            ImGui::InputText("Name", lightName, IM_ARRAYSIZE(lightName));
            ImGui::ColorEdit3("Color", col);
            ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 10.0f);

            if (currentLightType == LightType::Point) {
                ImGui::DragFloat3("Position", pos, 0.1f);
            }
            else if (currentLightType == LightType::Directional) {
                ImGui::DragFloat3("Direction", dir, 0.1f);
            }
            else if (currentLightType == LightType::Spot) {
                ImGui::DragFloat3("Position", pos, 0.1f);
                ImGui::DragFloat3("Direction", dir, 0.1f);
                ImGui::DragFloat("Cutoff", &cutoff, 0.01f, 0.0f, 1.0f);
                ImGui::DragFloat("Outer Cutoff", &outerCutoff, 0.01f, 0.0f, 1.0f);
            }

            if (ImGui::Button("Confirm", ImVec2(100, 30))) {
                std::string lname(lightName);
                Entity e = scene->AddLightEntity(lname, currentLightType,
                    glm::vec3(pos[0], pos[1], pos[2]),
                    glm::vec3(dir[0], dir[1], dir[2]),
                    glm::vec3(col[0], col[1], col[2]),
                    intensity, cutoff, outerCutoff);
                showLightProperties = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100, 30))) {
                showLightProperties = false;
            }
        }
    }


    ImGui::End();
}