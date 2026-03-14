// E:/PROJECTS/glfw/learning/project_FLAGE/engine/Editor/GUI/Panels/View_port_panel/FBO_taker/test_render_engine_FBO.cpp
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine_core.h" // must include glad first
#include "imgui.h"

#include "test_render_engine_FBO.h"

#include <iostream>
#include <limits>
#include <cstdint>
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\shadow_mapping.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\Editor\gizmos\gizmo_shader.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/test_render_engine.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/camera/camera_handeling/camera_handeling.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/scene/scene_class/scene.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/acpects/obj_selection/picking.h"

static GLint prevViewport[4] = {0,0,0,0};

FBO::FBO(int width, int height)
    : fbo(0), texture(0), rbo(0), width(width), height(height), hovered(false) {
    // Create framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Color texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Depth/stencil renderbuffer
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FBO:: Framebuffer is not complete!" << std::endl;
    }

    // Unbind to avoid accidental rendering to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FBO::~FBO() {
    if (rbo) { glDeleteRenderbuffers(1, &rbo); rbo = 0; }
    if (texture) { glDeleteTextures(1, &texture); texture = 0; }
    if (fbo) { glDeleteFramebuffers(1, &fbo); fbo = 0; }
}

void FBO::Bind() {
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
}

void FBO::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // restore previous viewport
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

void FBO::Resize(int newWidth, int newHeight) {
    if (newWidth <= 0 || newHeight <= 0) return;

    width = newWidth;
    height = newHeight;

    // Resize texture
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Resize renderbuffer
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}
gizmo_shader gizmoShader;
void FBO::RenderPanel(Camera& camera, test_render_engine& engine, Scene& scene) {
    ImGui::Begin("Viewport");
    ImVec2 panelSize = ImGui::GetContentRegionAvail();
    if (panelSize.x < 1 || panelSize.y < 1) { ImGui::End(); return; }

    if ((int)panelSize.x != width || (int)panelSize.y != height) Resize((int)panelSize.x, (int)panelSize.y);

    const glm::mat4 view = camera.GetViewMatrix();
    const float aspect = (float)width / (float)height;
    const glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, camera.NearPlane, camera.FarPlane);

    // 1. SHADOW PASS
    test_render_engine::shadow.cameraNear = camera.NearPlane;
    test_render_engine::shadow.cameraFar = camera.FarPlane;
    test_render_engine::shadow.RenderShadows(scene, view, projection,
        engine.cubeVAO, engine.planeVAO, engine.sphereVAO, engine.sphereIndexCount,
        engine.cylinderVAO, engine.cylinderIndexCount, engine.coneVAO, engine.coneIndexCount,
        engine.torusVAO, engine.torusIndexCount);

    // 2. BIND VIEWPORT FBO
    this->Bind();
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 3. MAIN RENDER
    static gizmo_shader gShader;
    static bool gInit = false;
    if (!gInit) { gShader.gizmo_init(); gInit = true; }
    engine.make_render(view, projection, camera.Position, scene, gShader, test_render_engine::shadow);

    this->Unbind();

    ImGui::Image((ImTextureID)(intptr_t)texture, panelSize, ImVec2(0, 1), ImVec2(1, 0));
    hovered = ImGui::IsWindowHovered();
    ImGui::End();
}