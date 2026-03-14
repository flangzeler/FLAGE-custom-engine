#pragma once
#include <glad/glad.h>
#include <string>

// Identify shader kind
enum class ShaderType {
    LEGACY,
    PBR
};

class Shader {
public:
    unsigned int ID;   // program ID
    ShaderType type;   // classification

    // Constructor with type tagging
    Shader(const char* vertexSource, const char* fragmentSource, ShaderType shaderType = ShaderType::LEGACY);

    void use() const { if (ID != 0) glUseProgram(ID); }

    // Utility functions for uniforms
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
};
