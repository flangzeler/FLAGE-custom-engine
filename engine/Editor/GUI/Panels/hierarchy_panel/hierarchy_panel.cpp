// hierarchy_panel.cpp
#include "hierarchy_panel.h"
#include "imgui.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/scene/scene_class/scene.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/scene/object_class/object.h"

// ------------------ Local helpers ------------------

static Entity* FindSceneEntityById(Scene* scene, uint32_t id) {
    for (auto& e : scene->entities) if (e.id == id) return &e;
    return nullptr;
}

static bool IsAncestor(Scene* scene, uint32_t ancestorId, uint32_t nodeId) {
    Entity* node = FindSceneEntityById(scene, nodeId);
    while (node && node->parent != -1) {
        if ((uint32_t)node->parent == ancestorId) return true;
        node = FindSceneEntityById(scene, (uint32_t)node->parent);
    }
    return false;
}

static std::string Trim(const std::string& s) {
    size_t start = 0, end = s.size();
    while (start < end && std::isspace((unsigned char)s[start])) ++start;
    while (end > start && std::isspace((unsigned char)s[end - 1])) --end;
    return s.substr(start, end - start);
}

// Ensure unique name in scene: if "Cube" exists, returns "Cube (2)", "Cube (3)", ...
static std::string EnsureUniqueName(Scene* scene, const std::string& base) {
    std::string root = Trim(base);
    if (root.empty()) root = "Entity";

    // Collect existing names
    std::vector<std::string> existing;
    existing.reserve(scene->entities.size());
    for (auto& e : scene->entities) {
        if (scene->world.hasName(e)) existing.push_back(scene->world.getName(e).name);
    }

    // If not present, return as-is
    if (std::find(existing.begin(), existing.end(), root) == existing.end()) {
        return root;
    }

    // Append (n) until unique
    int n = 2;
    while (true) {
        std::string candidate = root + " (" + std::to_string(n) + ")";
        if (std::find(existing.begin(), existing.end(), candidate) == existing.end()) {
            return candidate;
        }
        ++n;
    }
}

static bool PassesFilter(Scene* scene, Entity& eScene, const std::string& filter) {
    if (filter.empty()) return true;
    const auto& name = scene->world.getName(eScene).name;
    std::string a = name, b = filter;
    std::transform(a.begin(), a.end(), a.begin(), [](unsigned char c) { return std::tolower(c); });
    std::transform(b.begin(), b.end(), b.begin(), [](unsigned char c) { return std::tolower(c); });
    return a.find(b) != std::string::npos;
}

// ------------------ Panel state ------------------

struct RenameState {
    uint32_t editingId = UINT32_MAX;
    char buffer[256]{ 0 };
    bool requestFocus = false;
};
static RenameState gRename;

// ------------------ Node rendering ------------------

static void RenderEntityNode(Scene* scene, Entity& eScene, const std::string& filter) {
    // Filter: skip node if it doesn't match
    if (!PassesFilter(scene, eScene, filter)) return;

    auto& nameComp = scene->world.getName(eScene);
    int index = scene->FindIndexById(eScene.id);
    bool isSelected = (scene->selectedIndex == index);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth;
    if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;
    if (eScene.children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;

    // Use plain labels to avoid codepage warnings
    const char* icon = scene->hasLight(eScene) ? "Light" :
        scene->hasRender(eScene) ? "Render" : "Folder";

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);

    // Visibility checkbox before name
    if (scene->hasRender(eScene)) {
        auto& r = scene->world.getRender(eScene);
        std::string visId = "##vis_" + std::to_string(eScene.id);
        ImGui::Checkbox(visId.c_str(), &r.visible);
        ImGui::SameLine(0.0f, 6.0f);
    }

    bool opened = false;
    bool isEditing = (gRename.editingId == eScene.id);

    if (isEditing) {
        ImGui::PushID((int)eScene.id);
        if (gRename.requestFocus) {
            ImGui::SetKeyboardFocusHere();
            gRename.requestFocus = false;
        }
        // Inline rename field—commit on Enter, cancel on Escape
        if (ImGui::InputText("##rename", gRename.buffer, sizeof(gRename.buffer),
            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
            std::string newName = EnsureUniqueName(scene, gRename.buffer);
            scene->world.getName(eScene).name = newName;
            gRename.editingId = UINT32_MAX;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            gRename.editingId = UINT32_MAX;
        }
        ImGui::PopID();
        // Do not expand children while editing
    }
    else {
        // Normal node
        opened = ImGui::TreeNodeEx((void*)(intptr_t)eScene.id, flags, "%s %s", icon, nameComp.name.c_str());

        // Click selection
        if (ImGui::IsItemClicked()) {
            scene->selectedIndex = index;
            scene->selectedEntity = eScene;
        }
        // Double-click to rename
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            gRename.editingId = eScene.id;
            std::snprintf(gRename.buffer, sizeof(gRename.buffer), "%s", nameComp.name.c_str());
            gRename.requestFocus = true;
        }

        // Context menu
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Rename")) {
                gRename.editingId = eScene.id;
                std::snprintf(gRename.buffer, sizeof(gRename.buffer), "%s", nameComp.name.c_str());
                gRename.requestFocus = true;
            }
            if (ImGui::MenuItem("Duplicate")) {
                Entity dup = scene->DuplicateEntity(index);
                // Ensure unique name for duplicate (in case Scene::DuplicateEntity doesn't)
                auto& dupName = scene->world.getName(dup).name;
                dupName = EnsureUniqueName(scene, dupName);
                int dupIndex = scene->FindIndexById(dup.id);
                scene->selectedIndex = dupIndex;
                scene->selectedEntity = dup;
            }
            if (ImGui::MenuItem("Delete")) {
                scene->DeleteEntity(index);
                ImGui::EndPopup();
                return; // node removed
            }
            if (ImGui::MenuItem("Unparent")) {
                if (eScene.parent != -1) {
                    Entity* oldParent = scene->world.FindEntityById((uint32_t)eScene.parent);
                    if (oldParent) {
                        auto& kids = oldParent->children;
                        kids.erase(std::remove(kids.begin(), kids.end(), eScene.id), kids.end());
                    }
                    eScene.parent = -1;
                }
            }
            ImGui::EndPopup();
        }

        // Drag/drop parenting
        if (ImGui::BeginDragDropSource()) {
            uint32_t id = eScene.id;
            ImGui::SetDragDropPayload("ENTITY_DRAG_ID", &id, sizeof(uint32_t));
            ImGui::Text("Dragging %s", nameComp.name.c_str());
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG_ID")) {
                uint32_t draggedId = *(const uint32_t*)payload->Data;
                if (draggedId != eScene.id && !IsAncestor(scene, draggedId, eScene.id)) {
                    if (Entity* draggedScene = FindSceneEntityById(scene, draggedId)) {
                        scene->SetParent(*draggedScene, eScene);
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    // Children
    if (opened) {
        for (uint32_t childId : eScene.children) {
            if (Entity* childScene = FindSceneEntityById(scene, childId)) {
                RenderEntityNode(scene, *childScene, filter);
            }
        }
        ImGui::TreePop();
    }
}

// ------------------ Panel rendering ------------------

void HierarchyPanel::Render() {
    ImGui::Begin("Hierarchy");

    if (!scene) {
        ImGui::Text("No scene loaded.");
        ImGui::End();
        return;
    }

    // Search bar
    static char searchBuf[128]{ 0 };
    ImGui::InputTextWithHint("##search", "Search entities...", searchBuf, sizeof(searchBuf));
    std::string filter = searchBuf;

    ImGui::Separator();

    // Toolbar: Duplicate / Delete / Unparent
    bool hasSelection = (scene->selectedIndex >= 0 && scene->selectedIndex < (int)scene->entities.size());
    if (ImGui::Button("Duplicate") && hasSelection) {
        Entity dup = scene->DuplicateEntity(scene->selectedIndex);
        auto& dupName = scene->world.getName(dup).name;
        dupName = EnsureUniqueName(scene, dupName);
        int dupIndex = scene->FindIndexById(dup.id);
        scene->selectedIndex = dupIndex;
        scene->selectedEntity = dup;
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete") && hasSelection) {
        scene->DeleteEntity(scene->selectedIndex);
        if (gRename.editingId != UINT32_MAX) gRename.editingId = UINT32_MAX;
    }
   

    ImGui::Separator();

    // Table of hierarchy
    auto roots = scene->GetRootEntities();
    if (roots.empty()) {
        ImGui::TextDisabled("No entities in scene.");
    }
    else {
        if (ImGui::BeginTable("HierarchyTable", 1,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {

            for (Entity* rootScene : roots) {
                RenderEntityNode(scene, *rootScene, filter);
            }

            ImGui::EndTable();
        }
    }

    ImGui::End();
}
