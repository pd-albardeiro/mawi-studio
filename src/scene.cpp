#include "scene.hpp"

Scene make_default_scene() {
    Scene scene;
    scene.objects.push_back(SceneObject{});
    return scene;
}

SceneObject* selected_object(Scene& scene, int id) {
    for (auto& object : scene.objects) if (object.id == id) return &object;
    return nullptr;
}
