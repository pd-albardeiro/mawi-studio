#pragma once
#include "scene.hpp"
#include <filesystem>
#include <string>

bool import_obj(const std::filesystem::path& path, Scene& scene, int& selectedId, std::string& error);
bool export_obj(const std::filesystem::path& path, const SceneObject& object, std::string& error);
