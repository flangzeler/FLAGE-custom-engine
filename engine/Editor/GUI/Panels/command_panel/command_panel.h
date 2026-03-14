#pragma once
#include <imgui.h>
#include <string>
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/scene/scene_class/scene.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\scene\object_class\object.h"


class CreatePanel {
public:
    CreatePanel(Scene* scene);
    void Render();

private:
    Scene* scene;
    ShapeType currentShape = ShapeType::Cube; // track which shape to create
    bool showProperties = false;
};
