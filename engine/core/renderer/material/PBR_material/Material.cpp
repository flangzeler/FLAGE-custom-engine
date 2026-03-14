#include "Material.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\acpects\texture_loader\texture_loader.h" // make sure this is correct path
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\shaders\shaders_default.cpp"
static GLuint LoadIfValid(const std::string& path) {
    if (path.empty()) return 0;
    return loadTexture(path.c_str());
}

void Material::LoadTexturesIfNeeded() {
    if (useAlbedo && albedoTex == 0)
        albedoTex = LoadIfValid(albedoPath);

    if (useNormal && normalTex == 0)
        normalTex = LoadIfValid(normalPath);

    if (useMRAO && mraoTex == 0)
        mraoTex = LoadIfValid(mraoPath);

    if (!useMRAO) {
        if (useMetallicMap && metallicTex == 0)
            metallicTex = LoadIfValid(metallicPath);

        if (useRoughnessMap && roughnessTex == 0)
            roughnessTex = LoadIfValid(roughnessPath);

        if (useAOMap && aoTex == 0)
            aoTex = LoadIfValid(aoPath);
    }

    if (useOpacityMap && opacityTex == 0)
        opacityTex = LoadIfValid(opacityPath);

    if (useClearCoat && clearCoatTex == 0)
        clearCoatTex = LoadIfValid(clearCoatPath);

    if (useClearCoat && clearCoatRoughnessTex == 0)
        clearCoatRoughnessTex = LoadIfValid(clearCoatRoughnessPath);

    if (useEmissive && emissiveTex == 0)
        emissiveTex = LoadIfValid(emissivePath);
}

void Material::bind(const glm::vec3& viewPos) const {
    if (!shader || shader->ID == 0) return;

    shader->use();

    shader->setFloat("metallic", metallic);
    shader->setFloat("roughness", roughness);
    shader->setFloat("ao", ao);
    shader->setFloat("opacity", opacity);
    shader->setFloat("clearCoat", clearCoat);
    shader->setFloat("clearCoatRoughness", clearCoatRoughness);
    shader->setFloat("Emissive", Emissive);

    shader->setBool("useAlbedoMap", useAlbedo && albedoTex);
    shader->setBool("useNormalMap", useNormal && normalTex);
    shader->setBool("useMetallicMap", useMetallicMap && metallicTex);
    shader->setBool("useRoughnessMap", useRoughnessMap && roughnessTex);
    shader->setBool("useAOMap", useAOMap && aoTex);
    shader->setBool("useOpacityMap", useOpacityMap && opacityTex);
    shader->setBool("useClearCoatMap", useClearCoat && clearCoatTex);
    shader->setBool("useMRAO", useMRAO && mraoTex);
    shader->setBool("useIBL", useIBL);
    shader->setBool("useFog", useFog);

    shader->setInt("AlbedoMap", 0);
    shader->setInt("NormalMap", 1);
    shader->setInt("MetallicMap", 2);
    shader->setInt("RoughnessMap", 3);
    shader->setInt("AOMap", 4);
    shader->setInt("OpacityMap", 8);
    shader->setInt("ClearCoatMap", 9);
    shader->setInt("ClearCoatRoughnessMap", 11);
    shader->setInt("emissiveMap", 12);

    if (useAlbedo && albedoTex) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoTex);
    }
    if (useNormal && normalTex) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalTex);
    }
    if (useMetallicMap && metallicTex) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metallicTex);
    }
    if (useRoughnessMap && roughnessTex) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughnessTex);
    }
    if (useAOMap && aoTex) {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, aoTex);
    }
    if (useOpacityMap && opacityTex) {
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, opacityTex);
    }
    if (useClearCoat && clearCoatTex) {
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, clearCoatTex);
    }
    if (useClearCoat && clearCoatRoughnessTex) {
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, clearCoatRoughnessTex);
    }
    if (useEmissive && emissiveTex) {
        glActiveTexture(GL_TEXTURE12);
        glBindTexture(GL_TEXTURE_2D, emissiveTex);
    }
}
