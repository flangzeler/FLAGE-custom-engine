#include "inspector_panel.h"
#include "imgui.h"
#include <string>
#include <windows.h>
#undef APIENTRY
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\shaders\PBR_source\PBR_shader.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\scene\scene_class\scene.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/acpects/texture_loader/texture_loader.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\scene\object_class\object.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\shaders\PBR_source\IBL.h"

Ibl ibl;

// --- UTILITY: Native File Dialog ---
static bool OpenImageFileDialog(std::string& outPath) {
    OPENFILENAMEW ofn;
    wchar_t szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Image Files (*.png;*.jpg;*.jpeg;*.bmp)\0*.png;*.jpg;*.jpeg;*.bmp\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn) == TRUE) {
        char temp[260];
        size_t converted = 0;
        wcstombs_s(&converted, temp, sizeof(temp), szFile, _TRUNCATE);
        outPath = std::string(temp);
        return true;
    }
    return false;
}

// --- UI HELPER: Property Table Row ---
static void PropertyLabel(const char* label) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1);
}

// --- UI HELPER: Texture Slot with Native Loader (FIXED: Now saves path!) ---
static void TextureSlotUI(const char* label, GLuint& texHandle, std::string& texPath, bool& useFlag) {
    ImGui::PushID(label);
    PropertyLabel(label);

    ImGui::Checkbox("##use", &useFlag);
    ImGui::SameLine();

    char buf[128];
    if (texHandle == 0 && texPath.empty()) {
        sprintf_s(buf, "Empty (Click to Load)");
    }
    else if (texHandle == 0 && !texPath.empty()) {
        sprintf_s(buf, "Path saved: ...%s", texPath.substr(texPath.length() > 20 ? texPath.length() - 20 : 0).c_str());
    }
    else {
        sprintf_s(buf, "Tex ID: %u", texHandle);
    }

    if (ImGui::Button(buf, ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        std::string path;
        if (OpenImageFileDialog(path)) {
            GLuint newTex = loadTexture(path.c_str());
            if (newTex != 0) {
                texHandle = newTex;
                texPath = path;  // CRITICAL FIX: Save the path for serialization!
                useFlag = true;
            }
        }
    }
    ImGui::PopID();
}

void InspectorPanel::Render() {
    // Styling the window background
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.12f, 1.00f));
    ImGui::Begin("Inspector", NULL, ImGuiWindowFlags_NoCollapse);

    if (scene && scene->Selected()) {
        Entity e = *scene->Selected();
        auto& nameComp = scene->world.getName(e);

        // --- SECTION: ENTITY NAME ---
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.18f, 1.00f));
        static char nameBuffer[128];
        strncpy_s(nameBuffer, sizeof(nameBuffer), nameComp.name.c_str(), _TRUNCATE);
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##EntityName", nameBuffer, sizeof(nameBuffer))) {
            nameComp.name = std::string(nameBuffer);
        }
        ImGui::PopStyleColor();

        // --- SECTION: CHUNKY ACTION BUTTONS ---
        ImGui::Spacing();
        float availWidth = ImGui::GetContentRegionAvail().x;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float btnWidth = (availWidth - spacing) * 0.5f;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 10)); // Make them thick
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

        // Green Add Button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.40f, 0.22f, 1.0f));
        if (ImGui::Button("[+] COMPONENT", ImVec2(btnWidth, 0))) ImGui::OpenPopup("AddCompPopup");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // Red Remove Button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.40f, 0.22f, 0.22f, 1.0f));
        if (ImGui::Button("[-] REMOVE", ImVec2(btnWidth, 0))) ImGui::OpenPopup("RemCompPopup");
        ImGui::PopStyleColor();

        ImGui::PopStyleVar(2);

        // --- POPUP: Add Component (with Guards) ---
        if (ImGui::BeginPopup("AddCompPopup")) {
            bool hasLight = scene->hasLight(e);
            bool hasMaterial = scene->hasMaterial(e);

            if (ImGui::MenuItem("Transform", NULL, false, !scene->world.transforms.count(e.id)))
                scene->world.addTransform(e);

            if (ImGui::MenuItem("Render", NULL, false, !scene->hasRender(e)))
                scene->world.addRender(e);

            // Guard: Material and Light cannot coexist
            if (ImGui::MenuItem("Light", "[Source]", false, !hasMaterial)) {
                scene->world.addLight(e);
            }
            if (hasMaterial && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Remove Material component first to make this a Light.");

            if (ImGui::MenuItem("Material", "[Surface]", false, !hasLight)) {
                MaterialComponent matComp;
                static PBR_shader pbrSource;
                static Shader pbrShader(pbrSource.vertexSrc, pbrSource.fragmentSrc, ShaderType::PBR);
                matComp.material.shader = &pbrShader;
                scene->world.addMaterial(e, matComp);
            }
            if (hasLight && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Light sources cannot have PBR surface materials.");

            ImGui::EndPopup();
        }

        // --- POPUP: Remove Component ---
        if (ImGui::BeginPopup("RemCompPopup")) {
            if (scene->world.transforms.count(e.id) && ImGui::MenuItem("Transform")) scene->world.transforms.erase(e.id);
            if (scene->hasLight(e) && ImGui::MenuItem("Light")) scene->world.lights.erase(e.id);
            if (scene->hasMaterial(e) && ImGui::MenuItem("Material")) scene->world.materials.erase(e.id);
            if (scene->hasRender(e) && ImGui::MenuItem("Render")) scene->world.renders.erase(e.id);
            ImGui::EndPopup();
        }

        ImGui::Separator();
        ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody;

        // --- COMPONENT: TRANSFORM ---
        if (scene->world.transforms.count(e.id) && ImGui::CollapsingHeader("[#] Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::BeginTable("TTable", 2, tableFlags)) {
                ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthStretch);
                auto& t = scene->world.getTransform(e);
                PropertyLabel("Position"); ImGui::DragFloat3("##P", &t.position.x, 0.1f);
                PropertyLabel("Rotation"); ImGui::DragFloat3("##R", &t.rotation.x, 0.5f);
                PropertyLabel("Scale");    ImGui::DragFloat3("##S", &t.scale.x, 0.1f);
                ImGui::EndTable();
            }
        }

        // --- COMPONENT: LIGHT ---
        if (scene->hasLight(e) && ImGui::CollapsingHeader("[*] Light Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& l = scene->world.getLight(e);
            if (ImGui::BeginTable("LTable", 2, tableFlags)) {
                ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthStretch);

                PropertyLabel("Type");
                const char* types[] = { "Point", "Directional", "Spot" };
                int currentType = (int)l.type;
                if (ImGui::Combo("##LType", &currentType, types, 3)) l.type = (LightType)currentType;

                PropertyLabel("Color");     ImGui::ColorEdit3("##LC", &l.color.x);
                PropertyLabel("Intensity"); ImGui::DragFloat("##LI", &l.intensity, 0.5f, 0.0f, 1000.0f);

                if (l.type == LightType::Directional) {
                    PropertyLabel("Sun Dir");
                    if (ImGui::DragFloat3("##LDir", &l.direction.x, 0.01f)) l.direction = glm::normalize(l.direction);
                }
                else {
                    PropertyLabel("Position"); ImGui::DragFloat3("##LPos", &l.position.x, 0.1f);
                }
                ImGui::EndTable();
            }
        }

        // --- COMPONENT: MATERIAL ---
        if (scene->hasMaterial(e) && ImGui::CollapsingHeader("[O] PBR Material", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& mat = scene->world.getMaterial(e).material;
            if (ImGui::BeginTable("MTable", 2, tableFlags)) {
                ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthStretch);

                PropertyLabel("Base Color"); ImGui::ColorEdit3("##BC", &mat.baseColor.x);
                PropertyLabel("Metallic");   ImGui::SliderFloat("##Met", &mat.metallic, 0.0f, 1.0f);
                PropertyLabel("Roughness");  ImGui::SliderFloat("##Rou", &mat.roughness, 0.0f, 1.0f);
                PropertyLabel("Opacity");    ImGui::SliderFloat("##Opa", &mat.opacity, 0.0f, 1.0f);
                PropertyLabel("AO");         ImGui::SliderFloat("##ao", &mat.ao, 0.0f, 1.0f);
                PropertyLabel("Use IBL");    ImGui::Checkbox("##UseIBL", &mat.useIBL);
                PropertyLabel("Use Fog");    ImGui::Checkbox("##UseFog", &mat.useFog);
                PropertyLabel("Clear Coat"); ImGui::Checkbox("##UseCC", &mat.useClearCoat);

                if (mat.useClearCoat) {
                    PropertyLabel("  Factor");    ImGui::SliderFloat("##CCF", &mat.clearCoat, 0.0f, 1.0f);
                    PropertyLabel("  Roughness"); ImGui::SliderFloat("##CCR", &mat.clearCoatRoughness, 0.0f, 1.0f);
                }
                ImGui::EndTable();
            }

            ImGui::Spacing();
            if (ImGui::TreeNodeEx("Texture Maps", ImGuiTreeNodeFlags_Framed)) {
                if (ImGui::BeginTable("TexTable", 2, tableFlags)) {
                    ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                    ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthStretch);

                    // CRITICAL FIX: Now passing path references so they get saved!
                    TextureSlotUI("Albedo", mat.albedoTex, mat.albedoPath, mat.useAlbedo);
                    TextureSlotUI("Normal", mat.normalTex, mat.normalPath, mat.useNormal);
                    TextureSlotUI("Metallic", mat.metallicTex, mat.metallicPath, mat.useMetallicMap);
                    TextureSlotUI("Roughness", mat.roughnessTex, mat.roughnessPath, mat.useRoughnessMap);
                    TextureSlotUI("AO", mat.aoTex, mat.aoPath, mat.useAOMap);
                    TextureSlotUI("Opacity", mat.opacityTex, mat.opacityPath, mat.useOpacityMap);
                    TextureSlotUI("Clear coat", mat.clearCoatTex, mat.clearCoatPath, mat.useClearCoat);

                    ImGui::EndTable();
                }
                ImGui::TreePop();
            }
        }

    }
    else {
        // --- NO SELECTION: GLOBAL SETTINGS ---
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
        ImGui::TextDisabled("GLOBAL SCENE SETTINGS");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Load HDR Skybox", ImVec2(ImGui::GetContentRegionAvail().x * 0.7f, 35))) {
            std::string path;
            if (OpenImageFileDialog(path)) {
                ibl.initFromHDR(path.c_str(), "E:\\PROJECTS\\glfw\\learning\\project_FLAGE\\glsl cpp engine\\core\\renderer\\shaders\\source", 1048, 64, 256, 1048);

            }
        }
    }

    ImGui::End();
    ImGui::PopStyleColor(); // Pop WindowBg
}