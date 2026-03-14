#version 330 core
out vec4 FragColor;
in vec3 WorldPos;
uniform samplerCube environmentMap;
uniform float roughness;
void main() {
    // simplified for GT 610
    FragColor = textureLod(environmentMap, normalize(WorldPos), roughness * 4.0);
}