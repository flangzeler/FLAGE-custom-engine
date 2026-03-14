#pragma once
#include "scene.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/scene/scene_class/scene.h"

class MenuBar {
public:
    MenuBar(Scene* scene);

    void Render();

private:
    Scene* scene;
};
