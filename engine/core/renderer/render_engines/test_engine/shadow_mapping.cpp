#include "shadow_mapping.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\scene\scene_class\scene.h"

ShadowRenderer::ShadowRenderer(unsigned int shadowWidth,
    unsigned int shadowHeight,
    unsigned int pointShadowSize)
{
    SHADOW_WIDTH = shadowWidth;
    SHADOW_HEIGHT = shadowHeight;
    POINT_SHADOW_SIZE = pointShadowSize;

    shadowCascadeLevels = { 25.0f, 100.0f, 500.0f };
}

ShadowRenderer::~ShadowRenderer()
{
    if (depthShader2D) delete depthShader2D;

    for (auto& kv : shadow2DMaps) {
        glDeleteFramebuffers(1, &kv.second.fbo);
        glDeleteTextures(1, &kv.second.textureArray);
    }
}

void ShadowRenderer::InitShaders(unsigned int shadowWidth,
    unsigned int shadowHeight,
    unsigned int)
{
    SHADOW_WIDTH = shadowWidth;
    SHADOW_HEIGHT = shadowHeight;

    const char* vs = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 lightSpaceMatrix;
        uniform mat4 model;
        void main() {
            gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
        }
    )";

    const char* fs = R"(
        #version 330 core
        void main() { }
    )";

    if (depthShader2D) delete depthShader2D;
    depthShader2D = new Shader(vs, fs);
}

void ShadowRenderer::CreateCSMResources(unsigned int lightId)
{
    Shadow2D sd;

    glGenFramebuffers(1, &sd.fbo);
    glGenTextures(1, &sd.textureArray);

    glBindTexture(GL_TEXTURE_2D_ARRAY, sd.textureArray);
    glTexImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,
        GL_DEPTH_COMPONENT32F,
        SHADOW_WIDTH,
        SHADOW_HEIGHT,
        3,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float border[] = { 1,1,1,1 };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border);

    glBindFramebuffer(GL_FRAMEBUFFER, sd.fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sd.textureArray, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "CSM framebuffer incomplete\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shadow2DMaps[lightId] = sd;
}

glm::mat4 ShadowRenderer::CalculateLightSpaceMatrix(
    const glm::vec3& lightDir,
    int cascade)
{
    float splitFar = shadowCascadeLevels[cascade];
    float splitNear = (cascade == 0) ? cameraNear : shadowCascadeLevels[cascade - 1];

    float range = splitFar - splitNear;
    float radius = range * 0.6f;

    glm::vec3 lightPos = -glm::normalize(lightDir) * 300.0f;

    glm::mat4 view = glm::lookAt(lightPos, glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 proj = glm::ortho(-radius, radius, -radius, radius, -500.0f, 1000.0f);

    return proj * view;
}

void ShadowRenderer::RenderShadows(
    Scene& scene,
    const glm::mat4&,
    const glm::mat4&,
    unsigned int cubeVAO,
    unsigned int planeVAO,
    unsigned int sphereVAO,
    unsigned int sphereIdxCount,
    unsigned int cylinderVAO,
    unsigned int cylinderIdxCount,
    unsigned int coneVAO,
    unsigned int coneIdxCount,
    unsigned int torusVAO,
    unsigned int torusIdxCount)
{
    if (!depthShader2D) return;

    GLint oldVP[4];
    glGetIntegerv(GL_VIEWPORT, oldVP);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    for (auto& e : scene.entities) {
        if (!scene.world.hasLight(e)) continue;
        auto& light = scene.world.getLight(e);
        if (light.type != LightType::Directional) continue;

        if (!shadow2DMaps.count(e.id))
            CreateCSMResources(e.id);

        Shadow2D& sd = shadow2DMaps[e.id];

        for (int i = 0; i < 3; ++i)
            sd.lightSpaceMatrices[i] =
            CalculateLightSpaceMatrix(light.direction, i);

        glUseProgram(depthShader2D->ID);
        glBindFramebuffer(GL_FRAMEBUFFER, sd.fbo);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

        for (int c = 0; c < 3; ++c) {
            glFramebufferTextureLayer(
                GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT,
                sd.textureArray,
                0,
                c
            );
            glClear(GL_DEPTH_BUFFER_BIT);

            glUniformMatrix4fv(
                glGetUniformLocation(depthShader2D->ID, "lightSpaceMatrix"),
                1,
                GL_FALSE,
                glm::value_ptr(sd.lightSpaceMatrices[c])
            );

            for (auto& me : scene.entities) {
                if (!scene.world.hasRender(me)) continue;
                if (!scene.world.getRender(me).visible) continue;

                glm::mat4 model = scene.world.getModelMatrix(me);
                glUniformMatrix4fv(
                    glGetUniformLocation(depthShader2D->ID, "model"),
                    1,
                    GL_FALSE,
                    glm::value_ptr(model)
                );

                if (scene.world.hasModel(me)) {
                    auto& mc = scene.world.getModel(me);
                    glBindVertexArray(mc.VAO);
                    glDrawElements(GL_TRIANGLES,
                        (GLsizei)mc.indices.size(),
                        GL_UNSIGNED_INT,
                        0);
                }
                else {
                    auto s = scene.world.getRender(me).shape;
                    if (s == ShapeType::Cube) {
                        glBindVertexArray(cubeVAO);
                        glDrawArrays(GL_TRIANGLES, 0, 36);
                    }
                    else if (s == ShapeType::Plane) {
                        glBindVertexArray(planeVAO);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                    }
                    else if (s == ShapeType::Sphere) {
                        glBindVertexArray(sphereVAO);
                        glDrawElements(GL_TRIANGLES, sphereIdxCount, GL_UNSIGNED_INT, 0);
                    }
                    else if (s == ShapeType::Cylinder) {
                        glBindVertexArray(cylinderVAO);
                        glDrawElements(GL_TRIANGLES, cylinderIdxCount, GL_UNSIGNED_INT, 0);
                    }
                    else if (s == ShapeType::Cone) {
                        glBindVertexArray(coneVAO);
                        glDrawElements(GL_TRIANGLES, coneIdxCount, GL_UNSIGNED_INT, 0);
                    }
                    else if (s == ShapeType::Torus) {
                        glBindVertexArray(torusVAO);
                        glDrawElements(GL_TRIANGLES, torusIdxCount, GL_UNSIGNED_INT, 0);
                    }
                }
            }
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(oldVP[0], oldVP[1], oldVP[2], oldVP[3]);
    glCullFace(GL_BACK);
}

void ShadowRenderer::BindShadowMap(unsigned int lightId, unsigned int unit)
{
    auto it = shadow2DMaps.find(lightId);
    if (it == shadow2DMaps.end()) return;

    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, it->second.textureArray);
}

const glm::mat4* ShadowRenderer::GetLightSpaceMatrices(unsigned int lightId) const
{
    auto it = shadow2DMaps.find(lightId);
    return (it != shadow2DMaps.end()) ? it->second.lightSpaceMatrices : nullptr;
}

glm::mat4 ShadowRenderer::GetLightSpaceMatrix(unsigned int lightId) const
{
    auto it = shadow2DMaps.find(lightId);
    return (it != shadow2DMaps.end())
        ? it->second.lightSpaceMatrices[0]
        : glm::mat4(1.0f);
}
