#pragma once
#include <string>

class Ibl {
public:
    // Outputs used by your PBR shader
    unsigned int envCubemap = 0; // environment cubemap (RGB16F)
    unsigned int irradianceMap = 0; // diffuse IBL (RGB16F)
    unsigned int prefilteredEnvMap = 0; // specular IBL (RGB16F, mipmapped)
    unsigned int brdfLUT = 0; // BRDF integration LUT (RG16F)

    // Build IBL from HDR equirectangular texture and shader directory
    bool initFromHDR(const std::string& hdrPath,
        const std::string& shaderDir,
        int envSize = 256,
        int irradianceSize = 32,
        int prefilterSize = 64,
        int brdfSize = 256);

    // Bind for PBR draw (choose texture units)
    void bind(unsigned int irradianceUnit,
        unsigned int prefilterUnit,
        unsigned int brdfUnit) const;

    // Cleanup
    void destroy();

    // Temp resources
    unsigned int hdrTexture = 0;
    unsigned int captureFBO = 0, captureRBO = 0;
    unsigned int cubeVAO = 0, cubeVBO = 0;
    unsigned int quadVAO = 0, quadVBO = 0;

    // Programs
    unsigned int shEquirectToCube = 0;
    unsigned int shIrradiance = 0;
    unsigned int shPrefilter = 0;
    unsigned int shBrdfLUT = 0;

    // Internal steps
    bool loadHDR(const std::string& path);
    void createCubeMesh();
    void createQuadMesh();
    void createCaptureBuffers(int size);

    void createEnvCubemap(int size);
    void convertEquirectangularToCubemap(int size);

    void createIrradianceMap(int size);
    void convolveIrradiance(int size);

    void createPrefilterMap(int size);
    void prefilterEnv(int size);

    void createBrdfLUT(int size);
    void integrateBrdfLUT(int size);

    // Helpers
    static std::string loadFileAsString(const std::string& path);
    static unsigned int compileProgram(const char* vs, const char* fs);
    static unsigned int compileProgramFromFiles(const std::string& vsPath,
        const std::string& fsPath);

    void setCubeProjection(unsigned int program);
    void setCubeView(unsigned int program, int faceIndex);
};