#include "shaders_default.h"
#include <iostream>

// If glad is used as loader, the generated pointer variables live in glad.c
extern PFNGLCREATESHADERPROC glad_glCreateShader;
extern PFNGLCREATEPROGRAMPROC glad_glCreateProgram;

Shader::Shader(const char* vertexSource, const char* fragmentSource, ShaderType shaderType)
    : type(shaderType)
{
    if (glad_glCreateShader == nullptr || glad_glCreateProgram == nullptr) {
        std::cerr << "ERROR: OpenGL function pointers not initialized. "
                  << "Make sure 'glfwMakeContextCurrent(window)' is called before 'gladLoadGLLoader(...)'."
                  << std::endl;
        ID = 0;
        return;
    }

    int success;
    char infoLog[512];

    // Compile vertex shader
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, nullptr);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cerr << "ERROR::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Compile fragment shader
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, nullptr);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cerr << "ERROR::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Delete shaders after linking
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::setBool(const std::string& name, bool value) const {
    if (ID == 0) return;
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
    if (ID == 0) return;
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    if (ID == 0) return;
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
