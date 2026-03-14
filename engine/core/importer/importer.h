#pragma once
#include <string>
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/scene/scene_class/scene.h"

// Import a model file and create a single independent entity of type ShapeType::Model
class Importer {
public:
    static Entity ImportModel(Scene& scene, const std::string& path);

private:
    static std::string EnsureUniqueName(Scene& scene, const std::string& base);
};