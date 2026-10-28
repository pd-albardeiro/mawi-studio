#include "project.hpp"
#include <fstream>
#include <sstream>

bool save_project(const Scene& scene, const std::string& path, std::string& error) {
    std::ofstream out(path);
    if (!out) { error = "Could not write " + path; return false; }
    out << "MAWI_PROJECT 1\n";
    for (const auto& o : scene.objects) {
        out << "OBJECT " << o.id << ' ' << o.name << ' ' << o.type << ' '
            << o.position.x << ' ' << o.position.y << ' ' << o.position.z << ' '
            << o.rotation.x << ' ' << o.rotation.y << ' ' << o.rotation.z << ' '
            << o.scale.x << ' ' << o.scale.y << ' ' << o.scale.z << ' '
            << o.color.r << ' ' << o.color.g << ' ' << o.color.b << ' ' << o.parent << '\n';
    }
    return true;
}

bool load_project(Scene& scene, const std::string& path, std::string& error) {
    std::ifstream in(path);
    if (!in) { error = "Could not open " + path; return false; }
    std::string header; int version = 0;
    if (!(in >> header >> version) || header != "MAWI_PROJECT" || version != 1) {
        error = "Invalid or unsupported MAWI project"; return false;
    }
    Scene loaded; std::string tag;
    while (in >> tag) {
        if (tag != "OBJECT") { error = "Invalid project record"; return false; }
        SceneObject o;
        if (!(in >> o.id >> o.name >> o.type >> o.position.x >> o.position.y >> o.position.z
              >> o.rotation.x >> o.rotation.y >> o.rotation.z >> o.scale.x >> o.scale.y >> o.scale.z
              >> o.color.r >> o.color.g >> o.color.b >> o.parent)) {
            error = "Incomplete object record"; return false;
        }
        loaded.nextId = std::max(loaded.nextId, o.id + 1);
        loaded.objects.push_back(o);
    }
    if (loaded.objects.empty()) { error = "Project contains no objects"; return false; }
    scene = std::move(loaded);
    return true;
}
