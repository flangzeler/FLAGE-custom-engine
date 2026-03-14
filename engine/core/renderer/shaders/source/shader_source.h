#pragma once
#include <string>

class shader_source {
public:
    const char* vertexSrc = R"glsl(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor; 
        layout (location = 2) in vec2 aTexCoord;
        layout (location = 3) in vec3 aNormal;
        layout (location = 4) in vec3 aTangent; // for normal mapping

        uniform mat4 model;
        uniform mat4 MVP;
        uniform mat4 lightSpaceMatrix; 

        out vec3 FragPos;
        out vec3 Normal;
        out vec2 vTexCoord;
        out vec4 FragPosLightSpace; 
        out mat3 TBN; // tangent space basis

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
            vTexCoord = aTexCoord;

            // Offset along normal to reduce shadow acne
            vec3 shadowPosOffset = FragPos + (Normal * 0.08);
            FragPosLightSpace = lightSpaceMatrix * vec4(shadowPosOffset, 1.0);

            // Build TBN matrix for normal mapping
            vec3 T = normalize(mat3(model) * aTangent);
            vec3 N = normalize(Normal);
            vec3 B = normalize(cross(N, T));
            TBN = mat3(T, B, N);

            gl_Position = MVP * vec4(aPos, 1.0);
        }
    )glsl";

    const char* fragmentSrc = R"glsl(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 vTexCoord;
        in vec4 FragPosLightSpace;
        in mat3 TBN;

        uniform sampler2D Mytexture;
        uniform sampler2D NormalMap;   // NEW: normal map
        uniform sampler2D SpecularMap; // NEW: specular map
        uniform bool useTexture;
        uniform bool useNormalMap;
        uniform bool useSpecularMap;
        uniform vec3 viewPos;

        struct Light {
            int type; 
            vec3 position;
            vec3 direction;
            vec3 color;
            float intensity;
        };

        #define MAX_LIGHTS 8
        uniform int numLights;
        uniform Light lights[MAX_LIGHTS];
        uniform float ambientStrength;
        uniform float specularStrength;
        uniform float shininess;

        uniform sampler2D shadowMap;

        // Fog parameters
        uniform bool useFog;
        uniform vec3 fogColor;
        uniform float fogDensity;

        float CalcShadow(vec4 fragPosLS, vec3 norm, vec3 lightDir) {
            vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
            projCoords = projCoords * 0.5 + 0.5;
            if (projCoords.z > 1.0) return 0.0;

            float currentDepth = projCoords.z;
            float bias = max(0.005 * (1.0 - dot(norm, lightDir)), 0.0005);

            // 5x5 PCF kernel
            float shadow = 0.0;
            vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
            for (int x = -2; x <= 2; ++x) {
                for (int y = -2; y <= 2; ++y) {
                    float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
                    shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
                }
            }
            shadow /= 25.0;
            return shadow;
        }

        void main() {
            // Normal mapping
            vec3 norm = normalize(Normal);
            if(useNormalMap) {
                vec3 tangentNormal = texture(NormalMap, vTexCoord).rgb;
                tangentNormal = tangentNormal * 2.0 - 1.0; // [0,1] -> [-1,1]
                norm = normalize(TBN * tangentNormal);
            }

            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 objectColor = useTexture ? texture(Mytexture, vTexCoord).rgb : vec3(0.7);

            vec3 totalLighting = vec3(0.0);
            
            for(int i = 0; i < numLights && i < MAX_LIGHTS; i++) {
                vec3 lightDir = (lights[i].type == 1) ? normalize(-lights[i].direction) 
                                                     : normalize(lights[i].position - FragPos);
                
                float shadow = 0.0;
                if(lights[i].type == 1) {
                    shadow = CalcShadow(FragPosLightSpace, norm, lightDir);
                }

                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = diff * lights[i].color;

                vec3 halfwayDir = normalize(lightDir + viewDir);  
                float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess > 0.0 ? shininess : 32.0);

                float specStrength = specularStrength;
                if(useSpecularMap) {
                    specStrength *= texture(SpecularMap, vTexCoord).r;
                }

                vec3 specular = spec * lights[i].color * (specStrength / 10.0);

                totalLighting += (diffuse + specular) * lights[i].intensity * (1.0 - shadow);
            }

            vec3 ambient = (ambientStrength * 0.1) * objectColor;
            vec3 result = ambient + (totalLighting * objectColor);

            // Tone mapping + gamma correction
            result = result / (result + vec3(1.0));
            result = pow(result, vec3(1.0/2.2));

            // Fog
            if(useFog) {
                float distance = length(viewPos - FragPos);
                float fogFactor = exp(-pow(distance * fogDensity, 2.0));
                fogFactor = clamp(fogFactor, 0.0, 1.0);
                result = mix(fogColor, result, fogFactor);
            }

            FragColor = vec4(result, 1.0);
        }
    )glsl";
};
