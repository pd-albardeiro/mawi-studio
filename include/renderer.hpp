#pragma once
#include "scene.hpp"

class Renderer {
public:
    void orbit(float dx, float dy);
    void zoom(float amount);
    void draw(const Scene& scene, int selectedId, int x, int y, int width, int height, bool grid=true, bool axes=true);
    int pick(const Scene& scene, int x, int y, int width, int height) const;
private:
    float yaw_ = -32, pitch_ = 22, distance_ = 8.5f;
    void cube(const SceneObject& object, bool selected) const;
};
