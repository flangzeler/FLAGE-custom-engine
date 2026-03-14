#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <iostream>

class PBR_shader {
public:
    unsigned int ID = 0;
    void init() { ID = compileProgram(vertexSrc, fragmentSrc); }
    void use() const { if (ID != 0) glUseProgram(ID); }

    unsigned int compileProgram(const char* v, const char* f) {
        unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &v, nullptr);
        glCompileShader(vs);
        checkCompileErrors(vs, "VERTEX");

        unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &f, nullptr);
        glCompileShader(fs);
        checkCompileErrors(fs, "FRAGMENT");

        unsigned int p = glCreateProgram();
        glAttachShader(p, vs);
        glAttachShader(p, fs);
        glLinkProgram(p);
        checkCompileErrors(p, "PROGRAM");

        glDeleteShader(vs);
        glDeleteShader(fs);
        return p;
    }

private:
    void checkCompileErrors(unsigned int shader, std::string type) {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }

public:
    const char* vertexSrc = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 2) in vec2 aTexCoord;
    layout (location = 3) in vec3 aNormal;
    layout (location = 4) in vec3 aTangent;

    uniform mat4 model;
    uniform mat4 MVP;
    uniform mat4 view;
    uniform mat4 lightSpaceMatrices[3];

    out vec3 FragPos;
    out vec3 Normal;
    out vec2 vTexCoord;
    out mat3 TBN;
    out vec4 FragPosLightSpace[3];
    out float ViewSpaceDepth;

    void main() {
        vec4 worldPos = model * vec4(aPos, 1.0);
        FragPos = worldPos.xyz;
        vTexCoord = aTexCoord;
        Normal = normalize(mat3(transpose(inverse(model))) * aNormal);

        vec3 T = normalize(mat3(model) * aTangent);
        vec3 B = normalize(cross(Normal, T));
        TBN = mat3(T, B, Normal);

        for (int i = 0; i < 3; ++i)
            FragPosLightSpace[i] = lightSpaceMatrices[i] * worldPos;

        vec4 vPos = view * worldPos;
        ViewSpaceDepth = -vPos.z; 
        gl_Position = MVP * vec4(aPos, 1.0);
    }
    )glsl";

    const char* fragmentSrc = R"glsl(
    #version 330 core
    out vec4 FragColor;

    in vec3 FragPos;
    in vec3 Normal;
    in vec2 vTexCoord;
    in mat3 TBN;
    in vec4 FragPosLightSpace[3];
    in float ViewSpaceDepth;

    // Standard Maps
    uniform sampler2D AlbedoMap;
    uniform sampler2D NormalMap;
    uniform sampler2D MetallicMap;
    uniform sampler2D RoughnessMap;
    uniform sampler2D AOMap;
    uniform sampler2D OpacityMap;
    
    // Clear Coat Maps
    uniform sampler2D ClearCoatMap;
    uniform sampler2D ClearCoatRoughnessMap;

    uniform sampler2DArray shadowMap;
    uniform samplerCube irradianceMap;

    // Toggles
    uniform bool useAlbedoMap;
    uniform bool useNormalMap;
    uniform bool useMetallicMap;
    uniform bool useRoughnessMap;
    uniform bool useAOMap;
    uniform bool useOpacityMap;
    uniform bool useClearCoatMap;
    uniform bool useIBL;
    uniform bool useFog;

    // Base Values
    uniform vec3 baseColor;
    uniform vec3 viewPos;
    uniform float metallic;
    uniform float roughness;
    uniform float ao;
    uniform float clearCoat;
    uniform float clearCoatRoughness;
    uniform float opacity;

    // Fog Params
    uniform vec3 fogColor;
    uniform float fogDensity;

    uniform float cascadePlaneDistances[3];

    struct Light {
        int type;
        vec3 direction;
        vec3 color;
        float intensity;
    };
    uniform Light lights[4];
    uniform int numLights;

    const float PI = 3.14159265359;

    // --- PBR MATH ---
    float D_GGX(vec3 N, vec3 H, float r) {
        float a = r*r;
        float a2 = a*a;
        float NdotH = max(dot(N, H), 0.0);
        float NdotH2 = NdotH*NdotH;
        return a2 / (PI * pow(NdotH2 * (a2 - 1.0) + 1.0, 2.0));
    }

    float G_SchlickGGX(float NdotV, float r) {
        float k = pow(r + 1.0, 2.0) / 8.0;
        return NdotV / (NdotV * (1.0 - k) + k);
    }

    float G_Smith(vec3 N, vec3 V, vec3 L, float r) {
        return G_SchlickGGX(max(dot(N, V), 0.0), r) * G_SchlickGGX(max(dot(N, L), 0.0), r);
    }

    vec3 F_Schlick(float cosTheta, vec3 F0) {
        return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    }

    // --- CSM SHADOWS ---
    float CalcShadow(vec3 N, vec3 L) {
        int layer = 2;
        if (ViewSpaceDepth < cascadePlaneDistances[0]) layer = 0;
        else if (ViewSpaceDepth < cascadePlaneDistances[1]) layer = 1;

        vec3 projCoords = FragPosLightSpace[layer].xyz / FragPosLightSpace[layer].w;
        projCoords = projCoords * 0.5 + 0.5;
        if (projCoords.z > 1.0) return 0.0;



        float bias = max(0.005 * (1.0 - dot(N, L)), 0.0005);
        if (layer == 0) bias *= 0.2;

        float shadow = 0.0;
        vec2 texelSize = 1.5/ vec2(textureSize(shadowMap, 0));
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
                shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
            }
        }
        return shadow / 9.0;
    }

    void main() {
        // 1. Data Sampling
        float alpha = useOpacityMap ? texture(OpacityMap, vTexCoord).r : opacity;
        if(alpha < 0.05) discard; 

        vec3 albedo = (useAlbedoMap ? pow(texture(AlbedoMap, vTexCoord).rgb, vec3(2.2)) : vec3(1.0)) * baseColor;
        float m = useMetallicMap ? texture(MetallicMap, vTexCoord).r : metallic;
        float r = max(useRoughnessMap ? texture(RoughnessMap, vTexCoord).r : roughness, 0.05);
        float occ = useAOMap ? texture(AOMap, vTexCoord).r : ao;
        
        float cc = useClearCoatMap ? texture(ClearCoatMap, vTexCoord).r : clearCoat;
        float ccR = max(useClearCoatMap ? texture(ClearCoatRoughnessMap, vTexCoord).r : clearCoatRoughness, 0.05);

        vec3 N = normalize(Normal);
        if (useNormalMap)
            N = normalize(TBN * (texture(NormalMap, vTexCoord).rgb * 2.0 - 1.0));

        vec3 V = normalize(viewPos - FragPos);
        vec3 F0 = mix(vec3(0.04), albedo, m);

        // 2. Lighting Loop
        vec3 Lo = vec3(0.0);
        for(int i = 0; i < numLights && i < 4; ++i) {
            vec3 L = normalize(-lights[i].direction);
            vec3 H = normalize(V + L);
            vec3 radiance = lights[i].color * lights[i].intensity;

            float NDF = D_GGX(N, H, r);   
            float G   = G_Smith(N, V, L, r);      
            vec3 F    = F_Schlick(max(dot(H, V), 0.0), F0);
            
            vec3 numerator    = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            vec3 specular     = numerator / denominator;

            vec3 kS = F;
            vec3 kD = (vec3(1.0) - kS) * (1.0 - m);
            
            // Clear Coat
            float ccNDF = D_GGX(N, H, ccR);
            float ccG   = G_Smith(N, V, L, ccR);
            vec3 ccF    = F_Schlick(max(dot(H, V), 0.0), vec3(0.04)) * cc;
            vec3 ccSpecular = (ccNDF * ccG * ccF) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);

            float shadow = (i == 0) ? CalcShadow(N, L) : 0.0;
            float NdotL = max(dot(N, L), 0.0);
            Lo += (((kD * albedo / PI) + specular) * (1.0 - ccF) + ccSpecular) * radiance * NdotL * (1.0 - shadow);
        }

        // 3. Ambient
        vec3 ambient = vec3(0.03) * albedo * occ;
        if(useIBL) ambient = texture(irradianceMap, N).rgb * albedo * occ;

        vec3 color = ambient + Lo;

        // 4. Fog
        if(useFog) {
            float distance = length(viewPos - FragPos);
            float fogFactor = 1.0 / exp(distance * fogDensity);
            fogFactor = clamp(fogFactor, 0.0, 1.0);
            color = mix(fogColor, color, fogFactor);
        }

        // 5. Tone Map & Gamma
        color = color / (color + vec3(1.0));
        color = pow(color, vec3(1.0/2.2));

        FragColor = vec4(color, alpha);
    }
    )glsl";
};