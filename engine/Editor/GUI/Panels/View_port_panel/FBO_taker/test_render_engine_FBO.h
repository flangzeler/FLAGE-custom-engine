#pragma once
#ifndef FBO_H
#define FBO_H

#include <glad/glad.h>
#include <imgui.h>

// Forward declarations to avoid heavy includes and circular deps
class Camera;
class test_render_engine;
class Scene;

class FBO {
public:
    unsigned int fbo = 0;
    unsigned int texture = 0;
    unsigned int rbo = 0;
    int width = 0;
    int height = 0;
    bool hovered = false;

    FBO(int width, int height);
    ~FBO();

    // Call this once per frame
    void RenderPanel(Camera& camera, test_render_engine& engine, Scene& scene);
    bool IsHovered() const { return hovered; }

private:
    void Bind();
    void Unbind();
    void Resize(int newWidth, int newHeight);
};

#endif
