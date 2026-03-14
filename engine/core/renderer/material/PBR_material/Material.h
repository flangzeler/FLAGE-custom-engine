#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader;

class Material {
public:
    // ---- Shader ----
    Shader* shader = nullptr;

    // ---- Base PBR values ----
    glm::vec3 baseColor = glm::vec3(1.0f);
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ao = 1.0f;
    float opacity = 1.0f;

    // ---- Clear Coat ----
    bool useClearCoat = false;
    float clearCoat = 0.0f;
    float clearCoatRoughness = 0.03f;

    // ---- Emissive ----
    bool useEmissive = false;
    float Emissive = 0.0f;

    // ---- Fog / IBL ----
    bool useIBL = false;
    bool useFog = false;
    glm::vec3 fogColor = glm::vec3(0.5f);
    float fogDensity = 0.02f;

    // ---- Texture flags ----
    bool useAlbedo = false;
    bool useNormal = false;
    bool useMetallicMap = false;
    bool useRoughnessMap = false;
    bool useAOMap = false;
    bool useOpacityMap = false;
    bool useMRAO = false;

    // ---- Texture handles ----
    GLuint albedoTex = 0;
    GLuint normalTex = 0;
    GLuint metallicTex = 0;
    GLuint roughnessTex = 0;
    GLuint aoTex = 0;
    GLuint opacityTex = 0;
    GLuint mraoTex = 0;
    GLuint emissiveTex = 0;
    GLuint clearCoatTex = 0;
    GLuint clearCoatRoughnessTex = 0;

    // ---- Texture paths (for serialization) ----
    std::string albedoPath;
    std::string normalPath;
    std::string metallicPath;
    std::string roughnessPath;
    std::string aoPath;
    std::string opacityPath;
    std::string mraoPath;
    std::string emissivePath;
    std::string clearCoatPath;
    std::string clearCoatRoughnessPath;

    // ---- Core API ----
    void LoadTexturesIfNeeded();
    void bind(const glm::vec3& viewPos) const;
};
