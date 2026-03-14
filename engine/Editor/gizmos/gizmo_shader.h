#pragma once
#include <glad/glad.h>
#include <iostream>

class gizmo_shader {
public:
    const char* gizmoVertexShader = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    uniform mat4 MVP;
    void main() {
        gl_Position = MVP * vec4(aPos, 1.0);
    }
    )";

    const char* gizmoFragmentShader = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 gizmoColor;
    void main() {
        FragColor = vec4(gizmoColor, 1.0);
    }
    )";

    unsigned int programID;

    void gizmo_init();
    void use() const { glUseProgram(programID); }
    ~gizmo_shader() { glDeleteProgram(programID); }
};
