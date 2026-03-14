#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\render_engines\test_engine\scene\object_class\object.h"

class Scene {
public:
    World world;
    std::vector<Entity> entities;
    Entity selectedEntity{ UINT32_MAX };
    int selectedIndex = -1;

    Entity AddEntity(const std::string& name, ShapeType shape, const glm::vec3& color = glm::vec3(0.8f)) {
        Entity e = world.createEntity();
        world.addName(e, name);
        world.addTransform(e);
        world.addRender(e, RenderComponent(shape, color, true));
        world.addBounding(e, { 1.0f });
        entities.push_back(e);
        return e;
    }

    Entity AddLightEntity(const std::string& name,
        LightType type,
        const glm::vec3& position,
        const glm::vec3& direction,
        const glm::vec3& color,
        float intensity,
        float cutoff = glm::cos(glm::radians(12.5f)),
        float outerCutoff = glm::cos(glm::radians(17.5f))) {
        Entity e = world.createEntity();
        world.addName(e, name);

        LightComponent light;
        light.type = type;
        light.position = position;
        light.direction = direction;
        light.color = color;
        light.intensity = intensity;
        light.cutoff = cutoff;
        light.outerCutoff = outerCutoff;

        world.addLight(e, light);
        entities.push_back(e);
        return e;
    }

    Entity AddMaterialEntity(const std::string& name, const MaterialComponent& mat = {}) {
        Entity e = world.createEntity();
        world.addName(e, name);
        world.addTransform(e);
        world.addMaterial(e, mat);
        entities.push_back(e);
        return e;
    }

    bool hasRender(Entity e)    const { return world.hasRender(e); }
    bool hasLight(Entity e)     const { return world.hasLight(e); }
    bool hasMaterial(Entity e)  const { return world.hasMaterial(e); }  

    Entity* Selected() {
        return (selectedIndex >= 0 && selectedIndex < (int)entities.size())
            ? &entities[selectedIndex] : nullptr;
    }

    void SetParent(Entity& child, Entity& parent) { world.setParent(child, parent); }

    std::vector<Entity*> GetRootEntities() {
        std::vector<Entity*> roots;
        for (auto& e : entities) if (e.parent == -1) roots.push_back(&e);
        return roots;
    }

    std::vector<Entity*> GetChildren(Entity& parent) {
        std::vector<Entity*> kids;
        for (auto id : parent.children) {
            Entity* child = world.FindEntityById(id);
            if (child) kids.push_back(child);
        }
        return kids;
    }

    int FindIndexById(uint32_t id) const {
        for (int i = 0; i < (int)entities.size(); ++i)
            if (entities[i].id == id) return i;
        return -1;
    }

    void DeleteEntity(int index) {
        if (index < 0 || index >= (int)entities.size()) return;
        Entity e = entities[index];
        world.destroyEntity(e);
        entities.erase(entities.begin() + index);
        if (selectedIndex == index) {
            selectedIndex = -1;
            selectedEntity = { UINT32_MAX };
        }
    }

    Entity DuplicateEntity(int index) {
        if (index < 0 || index >= (int)entities.size()) return { UINT32_MAX };
        Entity src = entities[index];

        std::string srcName = world.getName(src).name;
        std::string newName = srcName + "_copy";

        Entity dup = world.createEntity();
        world.addName(dup, newName);

        if (world.hasTransform(src)) {
            auto t = world.getTransform(src);
            world.addTransform(dup, t);
        }
        if (world.hasRender(src)) {
            auto r = world.getRender(src);
            world.addRender(dup, r);
        }
        if (world.hasBounding(src)) {
            auto b = world.getBounding(src);
            world.addBounding(dup, b);
        }
        if (world.hasLight(src)) {
            auto l = world.getLight(src);
            world.addLight(dup, l);
        }
        if (world.hasMaterial(src)) {  
            auto m = world.getMaterial(src);
            world.addMaterial(dup, m);
        }

        entities.push_back(dup);
        return dup;
    }
};
