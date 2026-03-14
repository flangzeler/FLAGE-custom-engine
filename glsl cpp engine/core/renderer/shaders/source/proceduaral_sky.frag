#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform vec3 sunDir; // Pass your light direction here

void main() {
    vec3 dir = normalize(WorldPos);
    
    // Simple Gradient (Sky Blue to White at horizon)
    float height = dir.y;
    vec3 skyColor = vec3(0.2, 0.5, 0.9);
    vec3 horizonColor = vec3(0.7, 0.8, 1.0);
    vec3 color = mix(horizonColor, skyColor, max(height, 0.0));
    
    // Add a simple Sun Disk
    float sunSign = max(dot(dir, normalize(sunDir)), 0.0);
    float sunValue = pow(sunSign, 500.0) * 5.0; // Sun core
    float glowValue = pow(sunSign, 20.0) * 0.5; // Sun glow
    
    color += (vec3(1.0, 0.9, 0.7) * (sunValue + glowValue));
    
    FragColor = vec4(color, 1.0);
}