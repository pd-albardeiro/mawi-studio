#pragma once
#include "workspace.hpp"
#include <string>
#include <vector>

enum class HomeAction { Idle, Create, Open };
struct HomeState {
    std::vector<WorkspaceProject> recent;
    char name[81]{};
    char parent[1024]{};
    char openPath[1024]{};
    std::string error;
    bool newDialog=false,openDialog=false,help=false,settings=false,importPrompt=false;
    HomeAction action=HomeAction::Idle;
    std::string chosenPath;
};
void draw_home(HomeState& state);
void draw_help(bool& open);
