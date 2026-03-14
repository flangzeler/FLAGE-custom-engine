#version 330 core
out vec4 FragColor;
in vec3 WorldPos;
uniform sampler2D equirectMap;
void main() {
    vec3 v = normalize(WorldPos);
    vec2 uv = vec2(atan(v.z, v.x) * 0.1591 + 0.5, asin(v.y) * 0.3183 + 0.5);
    FragColor = vec4(texture(equirectMap, uv).rgb, 1.0);
}