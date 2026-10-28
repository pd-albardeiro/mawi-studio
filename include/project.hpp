#pragma once
#include "scene.hpp"
#include <string>

bool save_project(const Scene& scene, const std::string& path, std::string& error);
bool load_project(Scene& scene, const std::string& path, std::string& error);
