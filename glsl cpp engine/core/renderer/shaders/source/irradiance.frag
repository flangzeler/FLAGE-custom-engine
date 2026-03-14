#version 330 core
out vec4 FragColor;
in vec3 WorldPos;
uniform samplerCube environmentMap;
void main() {
    vec3 N = normalize(WorldPos);
    vec3 irradiance = vec3(0.0);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));
    float sampleDelta = 0.15; // Fewer samples for weak GPU
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 6.283; phi += sampleDelta) {
        for(float theta = 0.0; theta < 1.57; theta += sampleDelta) {
            vec3 temp = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = temp.x * right + temp.y * up + temp.z * N;
            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    FragColor = vec4(3.14159 * irradiance / nrSamples, 1.0);
}