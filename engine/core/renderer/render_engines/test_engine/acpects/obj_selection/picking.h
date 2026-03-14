// picking.h
#pragma once
#include <glm/glm.hpp>
#include<iostream>
inline glm::vec3 ScreenPointToRay(float mouseX, float mouseY, int screenW, int screenH,
    const glm::mat4& projection, const glm::mat4& view) {
    // Convert to Normalized Device Coordinates (NDC)
    float x = (2.0f * mouseX) / (float)screenW - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / (float)screenH;
    float z = -1.0f; // forward in NDC for a ray starting at near plane

    glm::vec4 rayClip(x, y, z, 1.0f);

    // Eye space
    glm::mat4 invProj = glm::inverse(projection);
    glm::vec4 rayEye = invProj * rayClip;
    rayEye.z = -1.0f;
    rayEye.w = 0.0f;

    // World space
    glm::mat4 invView = glm::inverse(view);
    glm::vec4 rayWorld = invView * rayEye;
    glm::vec3 rayDir = glm::normalize(glm::vec3(rayWorld));
   
    return rayDir;
}

inline bool RaySphereIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
    const glm::vec3& sphereCenter, float sphereRadius,
    float& outT) {
    glm::vec3 L = sphereCenter - rayOrigin;
    float tca = glm::dot(L, rayDir);
    float d2 = glm::dot(L, L) - tca * tca;
    float r2 = sphereRadius * sphereRadius;
    if (d2 > r2) return false;
    float thc = std::sqrt(r2 - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;

    // choose nearest positive t
    if (t0 > 0.0f) outT = t0;
    else if (t1 > 0.0f) outT = t1;
    else return false;

    return true;
}
