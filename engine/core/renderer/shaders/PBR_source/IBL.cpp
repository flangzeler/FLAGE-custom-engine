#include "Ibl.h"
#include <glad/glad.h>
#include <E:/PROJECTS/glfw/learning/project_FLAGE/Libraries/include/STB/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

bool Ibl::initFromHDR(const std::string& hdrPath, const std::string& shaderDir, int envSize, int irradianceSize, int prefilterSize, int brdfSize) {
    if (!loadHDR(hdrPath)) return false;
    shEquirectToCube = compileProgramFromFiles(shaderDir + "/cube.vert", shaderDir + "/equirect.frag");
    shIrradiance = compileProgramFromFiles(shaderDir + "/cube.vert", shaderDir + "/irradiance.frag");
    shPrefilter = compileProgramFromFiles(shaderDir + "/cube.vert", shaderDir + "/prefilter.frag");
    shBrdfLUT = compileProgramFromFiles(shaderDir + "/brdf.vert", shaderDir + "/brdf.frag");
    if (!shEquirectToCube || !shIrradiance || !shPrefilter || !shBrdfLUT) return false;

    createCubeMesh();
    createQuadMesh();
    createCaptureBuffers(envSize);
    createEnvCubemap(envSize);
    convertEquirectangularToCubemap(envSize);
    createIrradianceMap(irradianceSize);
    convolveIrradiance(irradianceSize);
    createPrefilterMap(prefilterSize);
    prefilterEnv(prefilterSize);
    createBrdfLUT(brdfSize);
    integrateBrdfLUT(brdfSize);


    return true;
}

void Ibl::bind(unsigned int iUnit, unsigned int pUnit, unsigned int bUnit) const {
    glActiveTexture(GL_TEXTURE0 + iUnit); glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glActiveTexture(GL_TEXTURE0 + pUnit); glBindTexture(GL_TEXTURE_CUBE_MAP, prefilteredEnvMap);
    glActiveTexture(GL_TEXTURE0 + bUnit); glBindTexture(GL_TEXTURE_2D, brdfLUT);
}

void Ibl::destroy() {
    glDeleteTextures(1, &envCubemap); glDeleteTextures(1, &irradianceMap);
    glDeleteTextures(1, &prefilteredEnvMap); glDeleteTextures(1, &brdfLUT);
    glDeleteVertexArrays(1, &cubeVAO); glDeleteBuffers(1, &cubeVBO);
}

bool Ibl::loadHDR(const std::string& path) {
    stbi_set_flip_vertically_on_load(true);
    int w, h, c; float* d = stbi_loadf(path.c_str(), &w, &h, &c, 0);
    if (!d) return false;
    glGenTextures(1, &hdrTexture); glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, d);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    stbi_image_free(d); return true;
}

void Ibl::createCubeMesh() {
    float v[] = { -1,-1,-1,1,-1,-1,1,1,-1,1,1,-1,-1,1,-1,-1,-1,-1,-1,-1,1,1,-1,1,1,1,1,1,1,1,-1,1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,-1,1,1,1,1,1,1,1,-1,1,-1,-1,1,-1,-1,1,-1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,-1,1,-1,1,1,1,1,1,1,1,-1,1,1,-1,1,-1 };
    glGenVertexArrays(1, &cubeVAO); glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO); glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, 0, 12, 0);
}

void Ibl::createQuadMesh() {
    float q[] = { -1,-1,1,-1,1,1,-1,1 }; unsigned int idx[] = { 0,1,2,0,2,3 };
    glGenVertexArrays(1, &quadVAO); glGenBuffers(1, &quadVBO);
    unsigned int ebo; glGenBuffers(1, &ebo);
    glBindVertexArray(quadVAO); glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(q), q, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT, 0, 8, 0);
}

void Ibl::createCaptureBuffers(int s) {
    glGenFramebuffers(1, &captureFBO); glGenRenderbuffers(1, &captureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO); glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, s, s);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
}

void Ibl::createEnvCubemap(int s) {
    glGenTextures(1, &envCubemap); glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (int i = 0; i < 6; ++i) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, s, s, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void Ibl::convertEquirectangularToCubemap(int s) {
    glUseProgram(shEquirectToCube); setCubeProjection(shEquirectToCube);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glViewport(0, 0, s, s); glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindVertexArray(cubeVAO);
    for (int i = 0; i < 6; ++i) {
        setCubeView(shEquirectToCube, i);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void Ibl::createIrradianceMap(int s) {
    glGenTextures(1, &irradianceMap); glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (int i = 0; i < 6; ++i) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, s, s, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void Ibl::convolveIrradiance(int s) {
    glUseProgram(shIrradiance); setCubeProjection(shIrradiance);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glViewport(0, 0, s, s); glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindVertexArray(cubeVAO);
    for (int i = 0; i < 6; ++i) {
        setCubeView(shIrradiance, i);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void Ibl::createPrefilterMap(int s) {
    glGenTextures(1, &prefilteredEnvMap); glBindTexture(GL_TEXTURE_CUBE_MAP, prefilteredEnvMap);
    for (int i = 0; i < 6; ++i) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, s, s, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void Ibl::prefilterEnv(int s) {
    glUseProgram(shPrefilter);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO); glBindVertexArray(cubeVAO);
    for (int m = 0; m < 5; ++m) {
        int mS = s >> m; glViewport(0, 0, mS, mS);
        glUniform1f(glGetUniformLocation(shPrefilter, "roughness"), (float)m / 4.0f);
        setCubeProjection(shPrefilter);
        for (int i = 0; i < 6; ++i) {
            setCubeView(shPrefilter, i);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilteredEnvMap, m);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
}

void Ibl::createBrdfLUT(int s) {
    glGenTextures(1, &brdfLUT); glBindTexture(GL_TEXTURE_2D, brdfLUT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, s, s, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void Ibl::integrateBrdfLUT(int s) {
    unsigned int f; glGenFramebuffers(1, &f); glBindFramebuffer(GL_FRAMEBUFFER, f);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUT, 0);
    glViewport(0, 0, s, s); glUseProgram(shBrdfLUT);
    glClear(GL_COLOR_BUFFER_BIT); glBindVertexArray(quadVAO); glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDeleteFramebuffers(1, &f);
}

std::string Ibl::loadFileAsString(const std::string& p) {
    std::ifstream f(p); std::ostringstream s; s << f.rdbuf(); return s.str();
}

unsigned int Ibl::compileProgram(const char* v, const char* f) {
    auto c = [](unsigned int t, const char* s) {
        unsigned int i = glCreateShader(t); glShaderSource(i, 1, &s, 0); glCompileShader(i); return i;
        };
    unsigned int vs = c(GL_VERTEX_SHADER, v), fs = c(GL_FRAGMENT_SHADER, f), p = glCreateProgram();
    glAttachShader(p, vs); glAttachShader(p, fs); glLinkProgram(p); return p;
}

unsigned int Ibl::compileProgramFromFiles(const std::string& v, const std::string& f) {
    return compileProgram(loadFileAsString(v).c_str(), loadFileAsString(f).c_str());
}

void Ibl::setCubeProjection(unsigned int p) {
    glm::mat4 m = glm::perspective(1.57f, 1.0f, 0.1f, 10.0f);
    glUniformMatrix4fv(glGetUniformLocation(p, "uProjection"), 1, 0, &m[0][0]);
}

void Ibl::setCubeView(unsigned int p, int i) {
    static const glm::mat4 v[6] = {
        glm::lookAt(glm::vec3(0), glm::vec3(1,0,0), glm::vec3(0,-1,0)),
        glm::lookAt(glm::vec3(0), glm::vec3(-1,0,0), glm::vec3(0,-1,0)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,1,0), glm::vec3(0,0,1)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,-1,0), glm::vec3(0,0,-1)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,0,1), glm::vec3(0,-1,0)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,0,-1), glm::vec3(0,-1,0))
    };
    glUniformMatrix4fv(glGetUniformLocation(p, "uView"), 1, 0, &v[i][0][0]);
}