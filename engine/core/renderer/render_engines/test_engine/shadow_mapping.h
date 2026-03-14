#pragma once
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\shaders\shaders_default.h"

struct Shadow2D {
    GLuint fbo = 0;
    GLuint textureArray = 0;
    glm::mat4 lightSpaceMatrices[3];
};

class Scene;

class ShadowRenderer {
public:
    unsigned int SHADOW_WIDTH;
    unsigned int SHADOW_HEIGHT;
    unsigned int POINT_SHADOW_SIZE;

    Shader* depthShader2D = nullptr;
    std::map<unsigned int, Shadow2D> shadow2DMaps;

    float cameraNear = 0.1f;
    float cameraFar = 500.0f;
    std::vector<float> shadowCascadeLevels;

    ShadowRenderer(unsigned int shadowWidth = 1024,
        unsigned int shadowHeight = 1024,
        unsigned int pointShadowSize = 512);
    ~ShadowRenderer();

    void InitShaders(unsigned int shadowWidth,
        unsigned int shadowHeight,
        unsigned int pointShadowSize);

    void RenderShadows(
        Scene& scene,
        const glm::mat4& viewMatrix,
        const glm::mat4& projectionMatrix,
        unsigned int cubeVAO,
        unsigned int planeVAO,
        unsigned int sphereVAO,
        unsigned int sphereIdxCount,
        unsigned int cylinderVAO,
        unsigned int cylinderIdxCount,
        unsigned int coneVAO,
        unsigned int coneIdxCount,
        unsigned int torusVAO,
        unsigned int torusIdxCount
    );

    void BindShadowMap(unsigned int lightId, unsigned int textureUnit);
    const glm::mat4* GetLightSpaceMatrices(unsigned int lightId) const;

    const std::vector<float>& GetCascadeLevels() const { return shadowCascadeLevels; }
    glm::mat4 GetLightSpaceMatrix(unsigned int lightId) const;

private:
    void CreateCSMResources(unsigned int lightId);
    glm::mat4 CalculateLightSpaceMatrix(const glm::vec3& lightDir, int cascadeIndex);
};
