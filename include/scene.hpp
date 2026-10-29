#pragma once
#include <string>
#include <vector>

struct Vec3 { float x = 0, y = 0, z = 0; };
struct Color { float r = .56f, g = .60f, b = .66f; };
struct Mesh {
    std::vector<Vec3> vertices;
    std::vector<std::vector<int>> faces;
};

struct SceneObject {
    int id = 1;
    std::string name = "Cube";
    std::string type = "Part";
    Vec3 position{0, .5f, 0};
    Vec3 rotation{0, 0, 0};
    Vec3 scale{1, 1, 1};
    Color color{.56f, .60f, .66f};
    int parent = 0;
    Vec3 pivot{0,0,0};
    Mesh mesh;
};

struct Scene { int nextId = 2; std::vector<SceneObject> objects; };
Scene make_default_scene();
SceneObject* selected_object(Scene& scene, int id);
Mesh cube_mesh();
bool extrude_face(Mesh& mesh, size_t faceIndex, float distance);
bool inset_face(Mesh& mesh, size_t faceIndex, float factor);
bool subdivide_face(Mesh& mesh, size_t faceIndex);
