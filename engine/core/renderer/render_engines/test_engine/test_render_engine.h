#pragma once
#ifndef TEST_RENDER_ENGINE_H
#define TEST_RENDER_ENGINE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\shadow_mapping.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\Editor\gizmos\gizmo_shader.h"
#include "scene/scene_class/scene.h"
#include "scene/object_class/object.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\shaders\shaders_default.h"
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\shaders\PBR_source\PBR_shader.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/shaders/source/shader_source.h" 

class test_render_engine {
public:
    GLuint shader_program = 0;
    GLuint texture = 0;
    GLuint defaultTexture = 0;
    Shader* shader = nullptr;

    GLuint cubeVAO = 0;
    GLuint cubeVBO = 0;
    GLuint cubeNormalVBO = 0;
    GLuint cubeTangentVBO = 0;

    GLuint planeVAO = 0;
    GLuint planeVBO = 0;
    GLuint planeEBO = 0;
    GLuint planeNormalVBO = 0;
    GLuint planeTangentVBO = 0;
    GLuint pyramidVAO = 0;
    GLuint pyramidVBO = 0;
    GLuint pyramidEBO = 0;
    GLuint pyramidNormalVBO = 0;
    GLuint pyramidTangentVBO = 0;
    GLuint sphereVAO = 0;
    GLuint sphereVBO = 0;
    GLuint sphereEBO = 0;
    unsigned int sphereIndexCount = 0;
    GLuint sphereTangentVBO = 0;

    GLuint cylinderVAO = 0;
    GLuint cylinderVBO = 0;
    GLuint cylinderEBO = 0;
    unsigned int cylinderIndexCount = 0;
    GLuint cylinderTangentVBO = 0;
    GLuint coneVAO = 0;
    GLuint coneVBO = 0;
    GLuint coneEBO = 0;
    unsigned int coneIndexCount = 0;
    GLuint coneTangentVBO = 0;
    GLuint torusVAO = 0;
    GLuint torusVBO = 0;
    GLuint torusEBO = 0;
    unsigned int torusIndexCount = 0;
    GLuint torusTangentVBO = 0;
    GLuint capsuleVAO = 0;
    GLuint capsuleVBO = 0;
    GLuint capsuleEBO = 0;
    unsigned int capsuleIndexCount = 0;
    GLuint capsuleTangentVBO = 0;
    GLuint gizmoQuadVAO = 0;
    GLuint gizmoQuadVBO = 0;

    void ready_render();
    void make_render(const glm::mat4& view,
        const glm::mat4& projection,
        const glm::vec3& camPos,
        Scene& scene,
        gizmo_shader& gizmoShader,
        ShadowRenderer& shadows);
    void initGizmoQuad();
    void end_render();
   static ShadowRenderer shadow;
};

#endif  