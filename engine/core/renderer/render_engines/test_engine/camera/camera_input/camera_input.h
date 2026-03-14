#pragma once
#ifndef CAMERA_INPUT_H
#define CAMERA_INPUT_H

// Forward declaration to avoid including GLFW in header
struct GLFWwindow;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\camera\camera_handeling\camera_handeling.h"

class CameraInput {
public:
    Camera* camera;
    float baseSpeed = 5.0f;   
    float speed = baseSpeed;

    float deltaTime;
    float lastFrame;

    bool firstMouse;
    float lastX;
    float lastY;

    glm::mat4 view;
    glm::mat4 projection;

    CameraInput(Camera* cam);
    bool allowInput = false; // <-- new flag void 
    void SetAllowInput(bool allow) { allowInput = allow; }
    // Call once per frame
    void Update(GLFWwindow* window, int screenWidth, int screenHeight);

    // Static callbacks
    static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    // Attach callbacks
    void AttachCallbacks(GLFWwindow* window);
};

#endif
