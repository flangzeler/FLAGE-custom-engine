// viewport.cpp — top of file
#include <E:\PROJECTS\glfw\learning\project_FLAGE\engine_core.h>  // MUST be first in any TU that uses OpenGL
#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\shadow_mapping.h"
#include <glm/gtc/matrix_transform.hpp>
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\Editor\gizmos\gizmo_shader.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/scene/scene_class/scene.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/camera/camera_handeling/camera_handeling.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/test_render_engine.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/Editor/GUI/Panels/View_port_panel/FBO_taker/test_render_engine_FBO.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/acpects/obj_selection/picking.h"
#include <iostream>
Camera camera;
void DrawMainViewport(FBO& fbo,
    Camera& camera,
    test_render_engine& renderer,
    Scene& scene)
{
    ImGui::Begin("Viewport");

    // --- Toolbar at top of viewport ---
    static int mode = 0; // 0 = Fill, 1 = Wireframe, 2 = Points

    ImGui::Text("Display Mode:");
    ImGui::SameLine();
    if (ImGui::RadioButton("Default", mode == 0)) mode = 0;
    ImGui::SameLine();
    if (ImGui::RadioButton("Wireframe", mode == 1)) mode = 1;
    ImGui::SameLine();
    if (ImGui::RadioButton("Points", mode == 2)) mode = 2;

    ImGui::Separator(); // visual break before image

    // Apply polygon mode BEFORE rendering
    switch (mode) {
    case 0: glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
    case 1: glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
    case 2: glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
    }

    // Build matrices
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.Zoom),
        (float)fbo.width / (float)fbo.height,
        0.1f, 100.0f
    );

    // Render scene
    static gizmo_shader gizmoShader;
    static bool gizmoInitialized = false;
    if (!gizmoInitialized) {
        gizmoShader.gizmo_init();
        gizmoInitialized = true;
    }

    test_render_engine::shadow.RenderShadows(scene,view,projection, renderer.cubeVAO, renderer.planeVAO, renderer.sphereVAO, renderer.sphereIndexCount, renderer.cylinderVAO, renderer.cylinderIndexCount, renderer.coneVAO, renderer.coneIndexCount, renderer.torusVAO, renderer.torusIndexCount);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderer.make_render(view, projection, camera.Position, scene, gizmoShader, test_render_engine::shadow);


    // Draw FBO texture
    ImVec2 imagePos = ImGui::GetCursorScreenPos();
    ImGui::Image((ImTextureID)(intptr_t)fbo.texture,
        ImVec2((float)fbo.width, (float)fbo.height),
        ImVec2(0, 1), ImVec2(1, 0));

    // Picking logic
    Entity picked = { UINT32_MAX };
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

        ImVec2 mouse = ImGui::GetMousePos();
        float mx = mouse.x - imagePos.x;
        float my = mouse.y - imagePos.y;

        if (mx >= 0.0f && my >= 0.0f && mx < (float)fbo.width && my < (float)fbo.height) {
            glm::vec3 camPos = camera.Position;
            glm::vec3 rayDir = ScreenPointToRay(mx, my, fbo.width, fbo.height, projection, view);

            float closestT = std::numeric_limits<float>::infinity();
            Entity picked = { UINT32_MAX };

            for (size_t i = 0; i < scene.entities.size(); ++i) {
                Entity e = scene.entities[i];
                auto& transform = scene.world.getTransform(e);
                auto& bound = scene.world.getBounding(e);

                float tHit;
                if (RaySphereIntersect(camPos, rayDir, transform.position, bound.radius, tHit)) {
                    if (tHit < closestT) {
                        closestT = tHit;
                        picked = e;
                        scene.selectedIndex = static_cast<int>(i);
                    }
                }
            }

            if (picked.id == UINT32_MAX) {
                scene.selectedIndex = -1;
            }
        }
    }
    // After picking logic
    for (size_t i = 0; i < scene.entities.size(); ++i) {
        Entity e = scene.entities[i];
        auto& render = scene.world.getRender(e);
        render.selected = (static_cast<int>(i) == scene.selectedIndex); 
        std::cout << "Picked ID: " << picked.id << std::endl;
    }

    ImGui::End();
}