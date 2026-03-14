#include <glad/glad.h>
#include <GLFW/glfw3.h>   // include only in cpp
#include "camera_input.h"

CameraInput::CameraInput(Camera* cam)
    : camera(cam), deltaTime(0.0f), lastFrame(0.0f),
    firstMouse(true), lastX(400.0f), lastY(300.0f) {
}
bool IsMouseHeld(GLFWwindow* window, int button) {
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

void CameraInput::Update(GLFWwindow* window, int screenWidth, int screenHeight) {
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;


   

    if (IsMouseHeld(window, GLFW_MOUSE_BUTTON_RIGHT)) {
        // Hide cursor only while RMB is held
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // WASD movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera->ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera->ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera->ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera->ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera->ProcessKeyboard(UPWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera->ProcessKeyboard(DOWNWARD, deltaTime);
    }
    else {
        // Restore cursor when RMB released
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    view = camera->GetViewMatrix();
    projection = glm::perspective(glm::radians(camera->Zoom),
        (float)screenWidth / (float)screenHeight,
        0.1f, 100.0f);
}


void CameraInput::MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    CameraInput* input = static_cast<CameraInput*>(glfwGetWindowUserPointer(window));
    if (!input || !input->camera) return;
    if (!input->allowInput) return;
        
    
    if (IsMouseHeld(window, GLFW_MOUSE_BUTTON_RIGHT)) {
        if (input->firstMouse) {
            input->lastX = (float)xpos;
            input->lastY = (float)ypos;
            input->firstMouse = false;
        }

        float xoffset = (float)xpos - input->lastX;
        float yoffset = input->lastY - (float)ypos;
        input->lastX = (float)xpos;
        input->lastY = (float)ypos;

        input->camera->ProcessMouseMovement(xoffset, yoffset);
    }
    else {
        input->firstMouse = true; // reset so next RMB press doesn’t jump
    }
}


void CameraInput::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    CameraInput* input = static_cast<CameraInput*>(glfwGetWindowUserPointer(window));
    if (!input || !input->camera) return;

    input->camera->ProcessMouseScroll((float)yoffset);
}

void CameraInput::AttachCallbacks(GLFWwindow* window) {
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
   // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
