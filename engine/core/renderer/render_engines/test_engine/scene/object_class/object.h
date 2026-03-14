#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <algorithm>

#include "shaders_default.h"
#include "E:\PROJECTS\glfw\learning\project_FLAGE\engine\core\renderer\material\PBR_material\Material.h"     

struct Entity {
    uint32_t id;
    int parent = -1;                         
    std::vector<uint32_t> children;      
};

enum class ShapeType { Cube, Sphere, Plane, Pyramid, Cylinder, Cone, Torus, Capsule, Model };

enum class LightType { Point, Directional, Spot };

struct NameComponent { std::string name; };

struct ModelComponent {
    std::string path;                  
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

struct LightComponent {
    LightType type = LightType::Point;
    glm::vec3 position{ 0.0f };
    glm::vec3 direction{ 0.0f, -1.0f, 0.0f };
    glm::vec3 color{ 1.0f };
    float intensity = 1.0f;

    float cutoff = glm::cos(glm::radians(12.5f));
    float outerCutoff = glm::cos(glm::radians(17.5f));
};

struct TransformComponent {
    glm::vec3 position{ 0.0f };
    glm::vec3 rotation{ 0.0f };
    glm::vec3 scale{ 1.0f };
};

struct RenderComponent {
    ShapeType shape;
    bool visible;
    bool useTexture;
    GLuint textureID;
    float ambientStrength;
    float specularStrength;
    float shininess;
    glm::vec3 baseColor;
    bool selected;
    std::string texturePath;

    RenderComponent(
        ShapeType s = ShapeType::Cube,
        glm::vec3 color = glm::vec3(0.8f),
        bool vis = true,
        bool tex = true
    )
        : shape(s), visible(vis), useTexture(tex), textureID(0),
        ambientStrength(0.2f), specularStrength(0.5f), shininess(32.0f),
        baseColor(color), selected(false) {
    }
};

struct BoundingComponent { float radius = 1.0f; };

struct MaterialComponent {
    Material material;     
};

class World {
public:
    Entity createEntity() {
        Entity e{ nextId++ };
        entities.push_back(e);
        return e;
    }

    void destroyEntity(Entity e) {
        if (e.parent != -1) {
            Entity* parent = FindEntityById((uint32_t)e.parent);
            if (parent) {
                auto& kids = parent->children;
                kids.erase(std::remove(kids.begin(), kids.end(), e.id), kids.end());
            }
        }
        for (auto childId : e.children) {
            Entity* child = FindEntityById(childId);
            if (child) child->parent = -1;
        }

        names.erase(e.id);
        transforms.erase(e.id);
        renders.erase(e.id);
        bounds.erase(e.id);
        lights.erase(e.id);
        models.erase(e.id);
        materials.erase(e.id);  

        entities.erase(std::remove_if(entities.begin(), entities.end(),
            [&](const Entity& ent) { return ent.id == e.id; }), entities.end());
    }

    void addName(Entity e, const std::string& name) { names[e.id] = { name }; }
    void addTransform(Entity e, const TransformComponent& t = {}) { transforms[e.id] = t; }
    void addRender(Entity e, const RenderComponent& r = {}) { renders[e.id] = r; }
    void addBounding(Entity e, const BoundingComponent& b = {}) { bounds[e.id] = b; }
    void addLight(Entity e, const LightComponent& l = {}) { lights[e.id] = l; }
    void addModel(Entity e, const ModelComponent& m = {}) { models[e.id] = m; }
    void addMaterial(Entity e, const MaterialComponent& mat = {}) { materials[e.id] = mat; } 

    NameComponent& getName(Entity e) { return names[e.id]; }
    TransformComponent& getTransform(Entity e) { return transforms[e.id]; }
    RenderComponent& getRender(Entity e) { return renders[e.id]; }
    BoundingComponent& getBounding(Entity e) { return bounds[e.id]; }
    LightComponent& getLight(Entity e) { return lights[e.id]; }
    ModelComponent& getModel(Entity e) { return models[e.id]; }
    MaterialComponent& getMaterial(Entity e) { return materials[e.id]; }  

    bool hasLight(Entity e)     const { return lights.find(e.id) != lights.end(); }
    bool hasRender(Entity e)    const { return renders.find(e.id) != renders.end(); }
    bool hasBounding(Entity e)  const { return bounds.find(e.id) != bounds.end(); }
    bool hasTransform(Entity e) const { return transforms.find(e.id) != transforms.end(); }
    bool hasName(Entity e)      const { return names.find(e.id) != names.end(); }
    bool hasModel(Entity e)     const { return models.find(e.id) != models.end(); }
    bool hasMaterial(Entity e)  const { return materials.find(e.id) != materials.end(); }  

    glm::mat4 getModelMatrix(Entity e) {
        auto it = transforms.find(e.id);
        glm::mat4 local(1.0f);

        if (it != transforms.end()) {
            auto& t = it->second;
            local = glm::translate(local, t.position);
            local = glm::rotate(local, glm::radians(t.rotation.x), { 1,0,0 });
            local = glm::rotate(local, glm::radians(t.rotation.y), { 0,1,0 });
            local = glm::rotate(local, glm::radians(t.rotation.z), { 0,0,1 });
            local = glm::scale(local, t.scale);
        }

        if (e.parent != -1) {
            Entity* parent = FindEntityById((uint32_t)e.parent);
            if (parent) return getModelMatrix(*parent) * local;
        }
        return local;
    }

    void setParent(Entity& child, Entity& parent) {
        if (child.id == parent.id) return;

        if (child.parent != -1) {
            Entity* oldParent = FindEntityById((uint32_t)child.parent);
            if (oldParent) {
                auto& kids = oldParent->children;
                kids.erase(std::remove(kids.begin(), kids.end(), child.id), kids.end());
            }
        }

        child.parent = (int)parent.id;

        if (std::find(parent.children.begin(), parent.children.end(), child.id) == parent.children.end()) {
            parent.children.push_back(child.id);
        }
    }

    Entity* FindEntityById(uint32_t id) {
        for (auto& e : entities) if (e.id == id) return &e;
        return nullptr;
    }

    uint32_t nextId = 0;
    std::vector<Entity> entities;

    std::unordered_map<uint32_t, NameComponent>      names;
    std::unordered_map<uint32_t, TransformComponent> transforms;
    std::unordered_map<uint32_t, RenderComponent>    renders;
    std::unordered_map<uint32_t, BoundingComponent>  bounds;
    std::unordered_map<uint32_t, LightComponent>     lights;
    std::unordered_map<uint32_t, ModelComponent>     models;
    std::unordered_map<uint32_t, MaterialComponent>  materials;  
};
