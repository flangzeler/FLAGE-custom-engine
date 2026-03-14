#pragma once
#include <string>
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/scene/scene_class/scene.h"

class SceneSerializer {
public:
    static void Save(Scene& scene, const std::string& path);
    static void Load(Scene& scene, const std::string& path);
};
