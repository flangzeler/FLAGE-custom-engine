#include "test_render_engine.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/shaders/shaders_default.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/shaders/source/shader_source.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/gui/windows/main_window/main_window.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/Libraries/include/STB/stb_image.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/scene/scene_class/scene.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\scene\object_class\object.h"
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\Editor\gizmos\gizmo_shader.h"
#include <E:\PROJECTS\glfw\learning\project_FLAGE\engine_core.h>
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\shadow_mapping.h"
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\shaders\PBR_source\IBL.h"
#include <GLFW/glfw3.h>
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\acpects\obj_selection\picking.cpp"
#include <iostream>
#include <filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
using namespace std;
ShadowRenderer test_render_engine::shadow;

void test_render_engine::initGizmoQuad() {
    float quadVertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
         0.5f,  0.5f, 0.0f
    };

    glGenVertexArrays(1, &this->gizmoQuadVAO);
    glGenBuffers(1, &this->gizmoQuadVBO);

    glBindVertexArray(this->gizmoQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->gizmoQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}
void test_render_engine::ready_render() {
    shader_source source;
    
    extern PFNGLCREATESHADERPROC glad_glCreateShader;
    if (glad_glCreateShader == NULL) {
        std::cerr << "ERROR: GL functions not initialized, aborting shader creation." << std::endl;
        return;    
    }
    Shader shader(source.vertexSrc, source.fragmentSrc);
    shader.use();
    this->shader_program = shader.ID;

    int width = 0, height = 0, nrChannel = 0;

    static const GLfloat cube_vertices[] = {
        -0.5f,-0.5f,-0.5f,  1,0,0,  0,0,
         0.5f,-0.5f,-0.5f,  0,1,0,  1,0,
         0.5f, 0.5f,-0.5f,  0,0,1,  1,1,
         0.5f, 0.5f,-0.5f,  0,0,1,  1,1,
        -0.5f, 0.5f,-0.5f,  1,1,0,  0,1,
        -0.5f,-0.5f,-0.5f,  1,0,0,  0,0,
        -0.5f,-0.5f, 0.5f,  1,0,1,  0,0,
         0.5f,-0.5f, 0.5f,  0,1,1,  1,0,
         0.5f, 0.5f, 0.5f,  1,1,1,  1,1,
         0.5f, 0.5f, 0.5f,  1,1,1,  1,1,
        -0.5f, 0.5f, 0.5f,  0.5f,0.5f,0.5f,  0,1,
        -0.5f,-0.5f, 0.5f,  1,0,1,  0,0,
        -0.5f, 0.5f, 0.5f,  0,1,0.5f,  1,0,
        -0.5f, 0.5f,-0.5f,  0.5f,0,1,  1,1,
        -0.5f,-0.5f,-0.5f,  0.5f,1,0,  0,1,
        -0.5f,-0.5f,-0.5f,  0.5f,1,0,  0,1,
        -0.5f,-0.5f, 0.5f,  0,0.5f,1,  0,0,
        -0.5f, 0.5f, 0.5f,  0,1,0.5f,  1,0,
         0.5f, 0.5f, 0.5f,  1,0.5f,0,  1,0,
         0.5f, 0.5f,-0.5f,  0,0.5f,1,  1,1,
         0.5f,-0.5f,-0.5f,  1,1,0.5f,  0,1,
         0.5f,-0.5f,-0.5f,  1,1,0.5f,  0,1,
         0.5f,-0.5f, 0.5f,  0.5f,0,0.5f,  0,0,
         0.5f, 0.5f, 0.5f,  1,0.5f,0,  1,0,
         -0.5f,-0.5f,-0.5f,  0.5f,0.5f,0,  0,1,
          0.5f,-0.5f,-0.5f,  0,0.5f,0.5f,  1,1,
          0.5f,-0.5f, 0.5f,  0.5f,0,0,    1,0,
          0.5f,-0.5f, 0.5f,  0.5f,0,0,    1,0,
         -0.5f,-0.5f, 0.5f,  0,0,0.5f,    0,0,
         -0.5f,-0.5f,-0.5f,  0.5f,0.5f,0,  0,1,
         -0.5f, 0.5f,-0.5f,  0.5f,0,0,    0,1,
          0.5f, 0.5f,-0.5f,  0,0.5f,0,    1,1,
          0.5f, 0.5f, 0.5f,  0,0,0.5f,    1,0,
          0.5f, 0.5f, 0.5f,  0,0,0.5f,    1,0,
         -0.5f, 0.5f, 0.5f,  0.5f,0.5f,1,  0,0,
         -0.5f, 0.5f,-0.5f,  0.5f,0,0,    0,1
    };

    static const GLfloat cube_normals[] = {
        0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1,
        0,0, 1, 0,0, 1, 0,0, 1, 0,0, 1, 0,0, 1, 0,0, 1,
       -1,0,0,-1,0,0,-1,0,0,-1,0,0,-1,0,0,-1,0,0,
       1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0,
       0,-1,0, 0,-1,0, 0,-1,0, 0,-1,0, 0,-1,0, 0,-1,0,
       0, 1,0, 0, 1,0, 0, 1,0, 0, 1,0, 0, 1,0, 0, 1,0
    };

    static const GLfloat cube_tangents[] = {
        1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0,
        1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0,
        0,0,1, 0,0,1, 0,0,1, 0,0,1, 0,0,1, 0,0,1,
        0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1,
        1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0,
        1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0
    };

    glGenVertexArrays(1, &this->cubeVAO);
    glGenBuffers(1, &this->cubeVBO);
    glGenBuffers(1, &this->cubeNormalVBO);
    glGenBuffers(1, &this->cubeTangentVBO);

    glBindVertexArray(this->cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, this->cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, this->cubeNormalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_normals), cube_normals, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, this->cubeTangentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tangents), cube_tangents, GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    float planeVertices[] = {
        -0.5f, 0.0f, -0.5f, 0.0f, 0.0f,
         0.5f, 0.0f, -0.5f, 1.0f, 0.0f,
         0.5f, 0.0f,  0.5f, 1.0f, 1.0f,
        -0.5f, 0.0f,  0.5f, 0.0f, 1.0f
    };
    unsigned int planeIndices[] = { 0,1,2, 0,2,3 };

    float planeNormals[] = {
        0,1,0,
        0,1,0,
        0,1,0,
        0,1,0
    };

    float planeTangents[] = {
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0
    };

    glGenVertexArrays(1, &this->planeVAO);
    glGenBuffers(1, &this->planeVBO);
    glGenBuffers(1, &this->planeEBO);
    glGenBuffers(1, &this->planeNormalVBO);
    glGenBuffers(1, &this->planeTangentVBO);

    glBindVertexArray(this->planeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, this->planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->planeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, this->planeNormalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeNormals), planeNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, this->planeTangentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeTangents), planeTangents, GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    float pyramidVertices[] = {
        -0.5f, 0.0f, -0.5f, 0.0f, 0.0f,
         0.5f, 0.0f, -0.5f, 1.0f, 0.0f,
         0.5f, 0.0f,  0.5f, 1.0f, 1.0f,
        -0.5f, 0.0f,  0.5f, 0.0f, 1.0f,
         0.0f,  1.0f,  0.0f, 0.5f, 0.5f
    };
    unsigned int pyramidIndices[] = {
        0,1,2, 0,2,3,        
        0,1,4, 1,2,4, 2,3,4, 3,0,4  
    };

    std::vector<glm::vec3> pyPos(5);
    for (int i = 0; i < 5; ++i)
        pyPos[i] = glm::vec3(pyramidVertices[i * 5 + 0],
            pyramidVertices[i * 5 + 1],
            pyramidVertices[i * 5 + 2]);

    std::vector<glm::vec3> pyNormals(5, glm::vec3(0.0f));
    for (size_t i = 0; i < sizeof(pyramidIndices) / sizeof(unsigned int); i += 3) {
        unsigned int i0 = pyramidIndices[i], i1 = pyramidIndices[i + 1], i2 = pyramidIndices[i + 2];
        glm::vec3 e1 = pyPos[i1] - pyPos[i0];
        glm::vec3 e2 = pyPos[i2] - pyPos[i0];
        glm::vec3 fn = glm::normalize(glm::cross(e1, e2));
        pyNormals[i0] += fn; pyNormals[i1] += fn; pyNormals[i2] += fn;
    }
    for (auto& n : pyNormals) n = glm::normalize(n);

    std::vector<glm::vec3> pyTangents(5, glm::vec3(0.0f));
    for (size_t i = 0; i < sizeof(pyramidIndices) / sizeof(unsigned int); i += 3) {
        unsigned int i0 = pyramidIndices[i], i1 = pyramidIndices[i + 1], i2 = pyramidIndices[i + 2];

        glm::vec3 p0 = pyPos[i0], p1 = pyPos[i1], p2 = pyPos[i2];
        glm::vec2 uv0(pyramidVertices[i0 * 5 + 3], pyramidVertices[i0 * 5 + 4]);
        glm::vec2 uv1(pyramidVertices[i1 * 5 + 3], pyramidVertices[i1 * 5 + 4]);
        glm::vec2 uv2(pyramidVertices[i2 * 5 + 3], pyramidVertices[i2 * 5 + 4]);

        glm::vec3 dp1 = p1 - p0;
        glm::vec3 dp2 = p2 - p0;
        glm::vec2 duv1 = uv1 - uv0;
        glm::vec2 duv2 = uv2 - uv0;

        float denom = duv1.x * duv2.y - duv2.x * duv1.y;
        float f = (fabs(denom) < 1e-8f) ? 0.0f : (1.0f / denom);

        glm::vec3 T = f * (dp1 * duv2.y - dp2 * duv1.y);
        if (glm::length(T) < 1e-8f) T = glm::vec3(1, 0, 0);

        pyTangents[i0] += T; pyTangents[i1] += T; pyTangents[i2] += T;
    }
    for (auto& t : pyTangents) t = glm::normalize(t);

    glGenVertexArrays(1, &this->pyramidVAO);
    glGenBuffers(1, &this->pyramidVBO);
    glGenBuffers(1, &this->pyramidEBO);
    glGenBuffers(1, &this->pyramidNormalVBO);
    glGenBuffers(1, &this->pyramidTangentVBO);

    glBindVertexArray(this->pyramidVAO);

    glBindBuffer(GL_ARRAY_BUFFER, this->pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->pyramidEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, this->pyramidNormalVBO);
    glBufferData(GL_ARRAY_BUFFER, pyNormals.size() * sizeof(glm::vec3), pyNormals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, this->pyramidTangentVBO);
    glBufferData(GL_ARRAY_BUFFER, pyTangents.size() * sizeof(glm::vec3), pyTangents.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    std::vector<glm::vec3> sphereTangents;

    const unsigned int X_SEGMENTS = 32;
    const unsigned int Y_SEGMENTS = 32;

    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);
            float yPos = std::cos(ySegment * M_PI);
            float zPos = std::sin(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);

            sphereVertices.push_back(xPos);
            sphereVertices.push_back(yPos);
            sphereVertices.push_back(zPos);

            sphereVertices.push_back(xPos);
            sphereVertices.push_back(yPos);
            sphereVertices.push_back(zPos);

            sphereVertices.push_back(xSegment);
            sphereVertices.push_back(ySegment);

            glm::vec3 tangent = glm::normalize(glm::vec3(
                -std::sin(xSegment * 2.0f * M_PI),
                0.0f,
                std::cos(xSegment * 2.0f * M_PI)
            ));
            sphereTangents.push_back(tangent);
        }
    }

    for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
        for (unsigned int x = 0; x < X_SEGMENTS; ++x) {
            unsigned int k1 = y * (X_SEGMENTS + 1) + x;
            unsigned int k2 = k1 + X_SEGMENTS + 1;

            sphereIndices.push_back(k1);
            sphereIndices.push_back(k2);
            sphereIndices.push_back(k1 + 1);

            sphereIndices.push_back(k1 + 1);
            sphereIndices.push_back(k2);
            sphereIndices.push_back(k2 + 1);
        }
    }

    sphereIndexCount = (unsigned int)sphereIndices.size();

    glGenVertexArrays(1, &this->sphereVAO);
    glGenBuffers(1, &this->sphereVBO);
    glGenBuffers(1, &this->sphereEBO);
    glGenBuffers(1, &this->sphereTangentVBO);

    glBindVertexArray(this->sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, this->sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, this->sphereTangentVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereTangents.size() * sizeof(glm::vec3), sphereTangents.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    std::vector<float> cylVertices;
    std::vector<unsigned int> cylIndices;
    std::vector<glm::vec3> cylTangents;

    const unsigned int CYL_SEGMENTS = 32;
    float cylHeight = 1.0f;
    float cylRadius = 0.5f;

    for (unsigned int i = 0; i <= CYL_SEGMENTS; ++i) {
        float theta = (float)i / CYL_SEGMENTS * 2.0f * M_PI;
        float nx = cos(theta);
        float nz = sin(theta);
        float x = nx * cylRadius;
        float z = nz * cylRadius;
        float u = (float)i / CYL_SEGMENTS;

        cylVertices.insert(cylVertices.end(), { x, 0.0f, z, nx, 0.0f, nz, u, 0.0f });
        cylVertices.insert(cylVertices.end(), { x, cylHeight, z, nx, 0.0f, nz, u, 1.0f });

        glm::vec3 tangent(-nz, 0.0f, nx);
        cylTangents.push_back(tangent);  
        cylTangents.push_back(tangent);  
    }
    for (unsigned int i = 0; i < CYL_SEGMENTS; ++i) {
        unsigned int start = i * 2;
        cylIndices.insert(cylIndices.end(), { start, start + 1, start + 2, start + 1, start + 3, start + 2 });
    }

    unsigned int topCenterIndex = cylVertices.size() / 8;
    cylVertices.insert(cylVertices.end(), { 0, cylHeight, 0, 0, 1, 0, 0.5f, 0.5f });
    cylTangents.push_back(glm::vec3(1, 0, 0));     
    for (unsigned int i = 0; i < CYL_SEGMENTS; ++i) {
        unsigned int ring = i * 2 + 1;
        cylIndices.insert(cylIndices.end(), { topCenterIndex, ring, ring + 2 });
    }

    unsigned int bottomCenterIndex = cylVertices.size() / 8;
    cylVertices.insert(cylVertices.end(), { 0, 0, 0, 0, -1, 0, 0.5f, 0.5f });
    cylTangents.push_back(glm::vec3(1, 0, 0));     
    for (unsigned int i = 0; i < CYL_SEGMENTS; ++i) {
        unsigned int ring = i * 2;
        cylIndices.insert(cylIndices.end(), { bottomCenterIndex, ring + 2, ring });
    }

    cylinderIndexCount = static_cast<unsigned int>(cylIndices.size());

    glGenVertexArrays(1, &this->cylinderVAO);
    glGenBuffers(1, &this->cylinderVBO);
    glGenBuffers(1, &this->cylinderEBO);
    glGenBuffers(1, &this->cylinderTangentVBO);

    glBindVertexArray(this->cylinderVAO);

    glBindBuffer(GL_ARRAY_BUFFER, this->cylinderVBO);
    glBufferData(GL_ARRAY_BUFFER, cylVertices.size() * sizeof(float), cylVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->cylinderEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylIndices.size() * sizeof(unsigned int), cylIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, this->cylinderTangentVBO);
    glBufferData(GL_ARRAY_BUFFER, cylTangents.size() * sizeof(glm::vec3), cylTangents.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    std::vector<float> coneVertices;
    std::vector<unsigned int> coneIndices;
    std::vector<glm::vec3> coneTangents;

    const unsigned int CONE_SEGMENTS = 32;
    float coneHeight = 1.0f;
    float coneRadius = 0.5f;

    coneVertices.insert(coneVertices.end(), { 0, coneHeight, 0, 0, 1, 0, 0.5f, 1.0f });
    coneTangents.push_back(glm::vec3(1, 0, 0));     

    for (unsigned int i = 0; i <= CONE_SEGMENTS; ++i) {
        float theta = (float)i / CONE_SEGMENTS * 2.0f * M_PI;
        float nx = cos(theta);
        float nz = sin(theta);
        float x = nx * coneRadius;
        float z = nz * coneRadius;
        float u = (float)i / CONE_SEGMENTS;

        coneVertices.insert(coneVertices.end(), { x, 0, z, nx, 0, nz, u, 0.0f });

        glm::vec3 tangent(-nz, 0.0f, nx);
        coneTangents.push_back(tangent);

        if (i < CONE_SEGMENTS) {
            coneIndices.insert(coneIndices.end(), { 0, i + 1, i + 2 });
        }
    }

    coneIndexCount = static_cast<unsigned int>(coneIndices.size());

   
    glGenVertexArrays(1, &this->coneVAO);
    glGenBuffers(1, &this->coneVBO);
    glGenBuffers(1, &this->coneEBO);
    glGenBuffers(1, &this->coneTangentVBO);

    glBindVertexArray(this->coneVAO);

 
    glBindBuffer(GL_ARRAY_BUFFER, this->coneVBO);
    glBufferData(GL_ARRAY_BUFFER, coneVertices.size() * sizeof(float), coneVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->coneEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, coneIndices.size() * sizeof(unsigned int), coneIndices.data(), GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, this->coneTangentVBO);
    glBufferData(GL_ARRAY_BUFFER, coneTangents.size() * sizeof(glm::vec3), coneTangents.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    std::vector<float> torusVertices;
    std::vector<unsigned int> torusIndices;
    std::vector<glm::vec3> torusTangents;

    const unsigned int TORUS_SEGMENTS = 32;
    const unsigned int TORUS_RINGS = 16;
    float R = 0.7f;
    float r = 0.3f;

    for (unsigned int i = 0; i <= TORUS_RINGS; ++i) {
        float phi = (float)i / TORUS_RINGS * 2.0f * M_PI;
        float cphi = cos(phi), sphi = sin(phi);
        for (unsigned int j = 0; j <= TORUS_SEGMENTS; ++j) {
            float theta = (float)j / TORUS_SEGMENTS * 2.0f * M_PI;
            float cth = cos(theta), sth = sin(theta);

            float x = (R + r * cth) * cphi;
            float y = (R + r * cth) * sphi;
            float z = r * sth;

 
            torusVertices.push_back(x);
            torusVertices.push_back(y);
            torusVertices.push_back(z);

 
            torusVertices.push_back(cth * cphi);
            torusVertices.push_back(cth * sphi);
            torusVertices.push_back(sth);

      
            torusVertices.push_back((float)j / TORUS_SEGMENTS);
            torusVertices.push_back((float)i / TORUS_RINGS);

          
            glm::vec3 tangent(-sth * cphi, -sth * sphi, cth);
            torusTangents.push_back(glm::normalize(tangent));
        }
    }
    for (unsigned int i = 0; i < TORUS_RINGS; ++i) {
        for (unsigned int j = 0; j < TORUS_SEGMENTS; ++j) {
            unsigned int first = i * (TORUS_SEGMENTS + 1) + j;
            unsigned int second = first + TORUS_SEGMENTS + 1;
            torusIndices.insert(torusIndices.end(), { first, second, first + 1, second, second + 1, first + 1 });
        }
    }
    torusIndexCount = static_cast<unsigned int>(torusIndices.size());

    glGenVertexArrays(1, &this->torusVAO);
    glGenBuffers(1, &this->torusVBO);
    glGenBuffers(1, &this->torusEBO);
    glGenBuffers(1, &this->torusTangentVBO);

    glBindVertexArray(this->torusVAO);

    glBindBuffer(GL_ARRAY_BUFFER, this->torusVBO);
    glBufferData(GL_ARRAY_BUFFER, torusVertices.size() * sizeof(float), torusVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->torusEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, torusIndices.size() * sizeof(unsigned int), torusIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, this->torusTangentVBO);
    glBufferData(GL_ARRAY_BUFFER, torusTangents.size() * sizeof(glm::vec3), torusTangents.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    std::vector<float> capsuleVertices;
    std::vector<unsigned int> capsuleIndices;
    std::vector<glm::vec3> capsuleTangents;

    const unsigned int CAP_SEGMENTS = 32;     
    const unsigned int CAP_RINGS = 16;         
    float capsuleRadius = 0.5f;
    float capsuleHeight = 1.0f;       

    for (unsigned int i = 0; i <= CAP_SEGMENTS; ++i) {
        float theta = (float)i / CAP_SEGMENTS * 2.0f * M_PI;
        float nx = cos(theta);
        float nz = sin(theta);
        float x = nx * capsuleRadius;
        float z = nz * capsuleRadius;
        float u = (float)i / CAP_SEGMENTS;

        capsuleVertices.insert(capsuleVertices.end(), { x, -capsuleHeight / 2.0f, z, nx, 0.0f, nz, u, 0.0f });
        capsuleVertices.insert(capsuleVertices.end(), { x,  capsuleHeight / 2.0f, z, nx, 0.0f, nz, u, 1.0f });

        glm::vec3 tangent(-nz, 0.0f, nx);
        capsuleTangents.push_back(tangent);  
        capsuleTangents.push_back(tangent);  
    }
    for (unsigned int i = 0; i < CAP_SEGMENTS; ++i) {
        unsigned int start = i * 2;
        capsuleIndices.insert(capsuleIndices.end(), {
            start, start + 1, start + 2,
            start + 1, start + 3, start + 2
            });
    }

    unsigned int topStart = capsuleVertices.size() / 8;
    for (unsigned int y = 0; y <= CAP_RINGS; ++y) {
        float phi = (float)y / CAP_RINGS * (M_PI / 2.0f);  
        float cphi = cos(phi), sphi = sin(phi);
        for (unsigned int x = 0; x <= CAP_SEGMENTS; ++x) {
            float theta = (float)x / CAP_SEGMENTS * 2.0f * M_PI;
            float cth = cos(theta), sth = sin(theta);

            float nx = cth * sphi;
            float ny = cphi;
            float nz = sth * sphi;

            float vx = nx * capsuleRadius;
            float vy = ny * capsuleRadius + capsuleHeight / 2.0f;
            float vz = nz * capsuleRadius;

            capsuleVertices.insert(capsuleVertices.end(), { vx, vy, vz, nx, ny, nz, (float)x / CAP_SEGMENTS, (float)y / CAP_RINGS });

            glm::vec3 tangent(-sth, 0.0f, cth);
            capsuleTangents.push_back(glm::normalize(tangent));
        }
    }
    for (unsigned int y = 0; y < CAP_RINGS; ++y) {
        for (unsigned int x = 0; x < CAP_SEGMENTS; ++x) {
            unsigned int first = topStart + y * (CAP_SEGMENTS + 1) + x;
            unsigned int second = first + CAP_SEGMENTS + 1;
            capsuleIndices.insert(capsuleIndices.end(), {
                first, second, first + 1,
                second, second + 1, first + 1
                });
        }
    }

 
    unsigned int bottomStart = capsuleVertices.size() / 8;
    for (unsigned int y = 0; y <= CAP_RINGS; ++y) {
        float phi = (float)y / CAP_RINGS * (M_PI / 2.0f);  
        float cphi = cos(phi), sphi = sin(phi);
        for (unsigned int x = 0; x <= CAP_SEGMENTS; ++x) {
            float theta = (float)x / CAP_SEGMENTS * 2.0f * M_PI;
            float cth = cos(theta), sth = sin(theta);

            float nx = cth * sphi;
            float ny = -cphi;
            float nz = sth * sphi;

            float vx = nx * capsuleRadius;
            float vy = ny * capsuleRadius - capsuleHeight / 2.0f;
            float vz = nz * capsuleRadius;

            capsuleVertices.insert(capsuleVertices.end(), { vx, vy, vz, nx, ny, nz, (float)x / CAP_SEGMENTS, (float)y / CAP_RINGS });

            glm::vec3 tangent(-sth, 0.0f, cth);
            capsuleTangents.push_back(glm::normalize(tangent));
        }
    }
    for (unsigned int y = 0; y < CAP_RINGS; ++y) {
        for (unsigned int x = 0; x < CAP_SEGMENTS; ++x) {
            unsigned int first = bottomStart + y * (CAP_SEGMENTS + 1) + x;
            unsigned int second = first + CAP_SEGMENTS + 1;
            capsuleIndices.insert(capsuleIndices.end(), {
                first, second, first + 1,
                second, second + 1, first + 1
                });
        }
    }

    capsuleIndexCount = static_cast<unsigned int>(capsuleIndices.size());

    glGenVertexArrays(1, &this->capsuleVAO);
    glGenBuffers(1, &this->capsuleVBO);
    glGenBuffers(1, &this->capsuleEBO);
    glGenBuffers(1, &this->capsuleTangentVBO);

    glBindVertexArray(this->capsuleVAO);

    
    glBindBuffer(GL_ARRAY_BUFFER, this->capsuleVBO);
    glBufferData(GL_ARRAY_BUFFER, capsuleVertices.size() * sizeof(float), capsuleVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->capsuleEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, capsuleIndices.size() * sizeof(unsigned int), capsuleIndices.data(), GL_STATIC_DRAW);

 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

   
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, this->capsuleTangentVBO);
    glBufferData(GL_ARRAY_BUFFER, capsuleTangents.size() * sizeof(glm::vec3), capsuleTangents.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

  
  
    this->initGizmoQuad();

}



static void DrawUsingExistingPath(const RenderComponent& render,
    GLuint cubeVAO, GLuint sphereVAO, GLuint planeVAO, GLuint pyramidVAO,
    GLuint cylinderVAO, GLuint coneVAO, GLuint torusVAO, GLuint capsuleVAO,
    unsigned int sphereIndexCount,
    unsigned int cylinderIndexCount,
    unsigned int coneIndexCount,
    unsigned int torusIndexCount,
    unsigned int capsuleIndexCount,
    World& world, Entity e,
    GLuint sceneShaderProgram,
    GLuint gizmoShaderProgram,
    const glm::mat4& view, const glm::mat4& projection,
    int lightIndex, int totalLights,unsigned int  gizmoQuadVAO)
{
    switch (render.shape) {
    case ShapeType::Cube:      glBindVertexArray(cubeVAO);     glDrawArrays(GL_TRIANGLES, 0, 36); break;
    case ShapeType::Sphere:    glBindVertexArray(sphereVAO);   glDrawElements(GL_TRIANGLE_STRIP, sphereIndexCount, GL_UNSIGNED_INT, 0); break;
    case ShapeType::Plane:     glBindVertexArray(planeVAO);    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); break;
    case ShapeType::Pyramid:   glBindVertexArray(pyramidVAO);  glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0); break;
    case ShapeType::Cylinder:  glBindVertexArray(cylinderVAO); glDrawElements(GL_TRIANGLES, cylinderIndexCount, GL_UNSIGNED_INT, 0); break;
    case ShapeType::Cone:      glBindVertexArray(coneVAO);     glDrawElements(GL_TRIANGLES, coneIndexCount, GL_UNSIGNED_INT, 0); break;
    case ShapeType::Torus:     glBindVertexArray(torusVAO);    glDrawElements(GL_TRIANGLES, torusIndexCount, GL_UNSIGNED_INT, 0); break;
    case ShapeType::Capsule:   glBindVertexArray(capsuleVAO);  glDrawElements(GL_TRIANGLES, capsuleIndexCount, GL_UNSIGNED_INT, 0); break;

    default:
        if (world.hasLight(e)) {
            const auto& l = world.getLight(e);

            
            if (lightIndex >= 0) {
                glUseProgram(sceneShaderProgram);
                std::string base = "lights[" + std::to_string(lightIndex) + "]";
                if (GLint uType = glGetUniformLocation(sceneShaderProgram, (base + ".type").c_str()); uType != -1)
                    glUniform1i(uType, (int)l.type);
                if (GLint uPos = glGetUniformLocation(sceneShaderProgram, (base + ".position").c_str()); uPos != -1)
                    glUniform3fv(uPos, 1, glm::value_ptr(l.position));
                if (GLint uDir = glGetUniformLocation(sceneShaderProgram, (base + ".direction").c_str()); uDir != -1)
                    glUniform3fv(uDir, 1, glm::value_ptr(l.direction));
                if (GLint uCol = glGetUniformLocation(sceneShaderProgram, (base + ".color").c_str()); uCol != -1)
                    glUniform3fv(uCol, 1, glm::value_ptr(l.color));
                if (GLint uInt = glGetUniformLocation(sceneShaderProgram, (base + ".intensity").c_str()); uInt != -1)
                    glUniform1f(uInt, l.intensity);
            }

            
            glUseProgram(gizmoShaderProgram);
            GLint mvpLoc = glGetUniformLocation(gizmoShaderProgram, "MVP");
            GLint colorLoc = glGetUniformLocation(gizmoShaderProgram, "gizmoColor");

            glm::mat4 model = glm::mat4(1.0f);
            if (l.type == LightType::Point) {
                model = glm::translate(model, l.position);
                model = glm::scale(model, glm::vec3(0.3f));
            }
            else if (l.type == LightType::Directional) {
                model = glm::translate(model, glm::vec3(0.0f));
                model = glm::scale(model, glm::vec3(0.5f));
            }
            else if (l.type == LightType::Spot) {
                model = glm::translate(model, l.position);
                model = glm::scale(model, glm::vec3(0.4f));
            }

            glm::mat4 mvp = projection * view * model;
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
            glUniform3fv(colorLoc, 1, glm::value_ptr(l.color));

            glBindVertexArray(gizmoQuadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
        break;
    }

    glBindVertexArray(0);
    glUseProgram(0);
}
static glm::mat4 alignPlusZToDir(const glm::vec3& dir) {
    glm::vec3 z = glm::normalize(dir);
    glm::vec3 up = (fabs(z.z) < 0.999f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 x = glm::normalize(glm::cross(up, z));
    glm::vec3 y = glm::cross(z, x);
    glm::mat4 rot(1.0f);
    rot[0] = glm::vec4(x, 0.0f);
    rot[1] = glm::vec4(y, 0.0f);
    rot[2] = glm::vec4(z, 0.0f);
    return rot;
}

extern Ibl ibl;
static void uploadLights(GLuint shaderProgram, Scene& scene, ShadowRenderer& shadows) {
    int lightCount = 0;
    for (Entity e : scene.entities) {
        if (!scene.world.hasLight(e)) continue;
        const auto& L = scene.world.getLight(e);

        std::string base = "lights[" + std::to_string(lightCount) + "]";
        glUniform1i(glGetUniformLocation(shaderProgram, (base + ".type").c_str()), (int)L.type);
        glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".position").c_str()), 1, glm::value_ptr(L.position));
        glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".direction").c_str()), 1, glm::value_ptr(L.direction));
        glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".color").c_str()), 1, glm::value_ptr(L.color));
        glUniform1f(glGetUniformLocation(shaderProgram, (base + ".intensity").c_str()), L.intensity);

        lightCount++;
        if (lightCount >= 4) break;
    }
    glUniform1i(glGetUniformLocation(shaderProgram, "numLights"), lightCount);
}

void test_render_engine::make_render(const glm::mat4& view,
    const glm::mat4& projection,
    const glm::vec3& camPos,
    Scene& scene,
    gizmo_shader& gizmoShader,
    ShadowRenderer& shadows)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

   
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int runningLightIndex = 0;

    for (Entity e : scene.entities) {
        if (!scene.world.hasRender(e)) continue;
        auto& render = scene.world.getRender(e);
        if (!render.visible) continue;

        glm::mat4 model = scene.world.getModelMatrix(e);

       
        if (scene.world.hasMaterial(e)) {
            auto& matComp = scene.world.getMaterial(e);
            Material& mat = matComp.material;

            if (mat.shader && mat.shader->ID != 0) {
                glUseProgram(mat.shader->ID);

              
                glUniform1fv(glGetUniformLocation(mat.shader->ID, "cascadePlaneDistances"), 3, shadows.shadowCascadeLevels.data());

                for (Entity le : scene.entities) {
                    if (scene.world.hasLight(le)) {
                        auto& light = scene.world.getLight(le);
                        if (light.type == LightType::Directional) {
                            const glm::mat4* matrices = shadows.GetLightSpaceMatrices(le.id);
                            if (matrices) {
                                glUniformMatrix4fv(glGetUniformLocation(mat.shader->ID, "lightSpaceMatrices"), 3, GL_FALSE, glm::value_ptr(matrices[0]));
                                shadows.BindShadowMap(le.id, 10);
                                glUniform1i(glGetUniformLocation(mat.shader->ID, "shadowMap"), 10);
                                break;
                            }
                        }
                    }
                }

              
                glUniform3fv(glGetUniformLocation(mat.shader->ID, "viewPos"), 1, glm::value_ptr(camPos));
                uploadLights(mat.shader->ID, scene, shadows);

                glUniformMatrix4fv(glGetUniformLocation(mat.shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glUniformMatrix4fv(glGetUniformLocation(mat.shader->ID, "MVP"), 1, GL_FALSE, glm::value_ptr(projection * view * model));

                if (ibl.irradianceMap && ibl.prefilteredEnvMap && ibl.brdfLUT) {
                    glUniform1i(glGetUniformLocation(mat.shader->ID, "irradianceMap"), 5);
                    glUniform1i(glGetUniformLocation(mat.shader->ID, "prefilteredEnvMap"), 6);
                    glUniform1i(glGetUniformLocation(mat.shader->ID, "brdfLUT"), 7);
                    glUniform1i(glGetUniformLocation(mat.shader->ID, "useIBL"), mat.useIBL ? 1 : 0);
                    ibl.bind(5, 6, 7);
                }

                
                glUniform1i(glGetUniformLocation(mat.shader->ID, "useAlbedoMap"), mat.useAlbedo ? 1 : 0);
                glUniform1i(glGetUniformLocation(mat.shader->ID, "useNormalMap"), mat.useNormal ? 1 : 0);
                glUniform1i(glGetUniformLocation(mat.shader->ID, "useMetallicMap"), mat.useMetallicMap ? 1 : 0);
                glUniform1i(glGetUniformLocation(mat.shader->ID, "useRoughnessMap"), mat.useRoughnessMap ? 1 : 0);
                glUniform1i(glGetUniformLocation(mat.shader->ID, "useAOMap"), mat.useAOMap ? 1 : 0);
                glUniform1i(glGetUniformLocation(mat.shader->ID, "useOpacityMap"), mat.useOpacityMap ? 1 : 0);
                glUniform1i(glGetUniformLocation(mat.shader->ID, "useClearCoatMap"), mat.useClearCoat ? 1 : 0);

              
                glUniform3fv(glGetUniformLocation(mat.shader->ID, "baseColor"), 1, glm::value_ptr(mat.baseColor));
                glUniform1f(glGetUniformLocation(mat.shader->ID, "metallic"), mat.metallic);
                glUniform1f(glGetUniformLocation(mat.shader->ID, "roughness"), mat.roughness);
                glUniform1f(glGetUniformLocation(mat.shader->ID, "ao"), mat.ao);
                glUniform1f(glGetUniformLocation(mat.shader->ID, "clearCoat"), mat.clearCoat);
                glUniform1f(glGetUniformLocation(mat.shader->ID, "clearCoatRoughness"), mat.clearCoatRoughness);
                glUniform1f(glGetUniformLocation(mat.shader->ID, "opacity"), mat.opacity);

               
                mat.bind(camPos);

               
                if (render.shape == ShapeType::Model && scene.world.hasModel(e)) {
                    auto& mc = scene.world.getModel(e);
                    glBindVertexArray(mc.VAO);
                    glDrawElements(GL_TRIANGLES, (int)mc.indices.size(), GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);
                }
                else {
                    int idx = scene.world.hasLight(e) ? runningLightIndex : -1;
                    DrawUsingExistingPath(render, this->cubeVAO, this->sphereVAO, this->planeVAO, this->pyramidVAO,
                        this->cylinderVAO, this->coneVAO, this->torusVAO, this->capsuleVAO,
                        this->sphereIndexCount, this->cylinderIndexCount, this->coneIndexCount,
                        this->torusIndexCount, this->capsuleIndexCount, scene.world, e,
                        mat.shader->ID, gizmoShader.programID, view, projection,
                        idx, 8, this->gizmoQuadVAO);
                    if (scene.world.hasLight(e)) runningLightIndex++;
                }
            }
        }
    
        else {
            glUseProgram(this->shader_program);
            uploadLights(this->shader_program, scene, shadows);
            glUniform3fv(glGetUniformLocation(this->shader_program, "viewPos"), 1, glm::value_ptr(camPos));

            bool hasTex = (render.useTexture && render.textureID != 0);
            glUniform1i(glGetUniformLocation(this->shader_program, "useTexture"), hasTex ? 1 : 0);
            if (hasTex) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, render.textureID);
                glUniform1i(glGetUniformLocation(this->shader_program, "Mytexture"), 0);
            }

            glUniformMatrix4fv(glGetUniformLocation(this->shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(this->shader_program, "MVP"), 1, GL_FALSE, glm::value_ptr(projection * view * model));

            if (render.shape == ShapeType::Model && scene.world.hasModel(e)) {
                auto& mc = scene.world.getModel(e);
                glBindVertexArray(mc.VAO);
                glDrawElements(GL_TRIANGLES, (int)mc.indices.size(), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
            else {
                int idx = scene.world.hasLight(e) ? runningLightIndex : -1;
                DrawUsingExistingPath(render, this->cubeVAO, this->sphereVAO, this->planeVAO, this->pyramidVAO,
                    this->cylinderVAO, this->coneVAO, this->torusVAO, this->capsuleVAO,
                    this->sphereIndexCount, this->cylinderIndexCount, this->coneIndexCount,
                    this->torusIndexCount, this->capsuleIndexCount, scene.world, e,
                    this->shader_program, gizmoShader.programID, view, projection,
                    idx, 8, this->gizmoQuadVAO);
                if (scene.world.hasLight(e)) runningLightIndex++;
            }
        }
    }
    glDisable(GL_BLEND); 
}