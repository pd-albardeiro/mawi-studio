#pragma once
#include "scene.hpp"
#include <filesystem>
#include <string>
#include <vector>

struct WorkspaceProject {
    std::filesystem::path directory;
    std::string name;
    std::string created;
    std::string lastOpened;
};

std::filesystem::path recent_file();
std::vector<WorkspaceProject> read_recent();
bool remember_project(const WorkspaceProject& project, std::string& error);
bool forget_project(const std::filesystem::path& directory, std::string& error);
bool create_workspace(const std::filesystem::path& parent, const std::string& name,
                      WorkspaceProject& project, Scene& scene, std::string& error);
bool open_workspace(const std::filesystem::path& directory, WorkspaceProject& project,
                    Scene& scene, std::string& error);
bool save_workspace(const WorkspaceProject& project, const Scene& scene, std::string& error);
