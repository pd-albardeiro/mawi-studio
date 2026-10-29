#include "home.hpp"
#include "imgui.h"
#include "brand.hpp"
#include <cstdio>
#include <cstring>

namespace {
std::string choose_folder(){
    FILE* pipe=popen("zenity --file-selection --directory --title='Choose project folder' 2>/dev/null","r");
    if(!pipe)return {};
    char buf[2048];std::string result;
    while(fgets(buf,sizeof(buf),pipe))result+=buf;
    pclose(pipe);while(!result.empty()&&(result.back()=='\n'||result.back()=='\r'))result.pop_back();
    return result;
}
void copy(char* dest,size_t size,const std::string& value){std::snprintf(dest,size,"%s",value.c_str());}
}

void draw_help(bool& open){
    if(!open)return;
    ImGui::SetNextWindowSize(ImVec2(760,580),ImGuiCond_FirstUseEver);
    if(!ImGui::Begin("Help & Documentation",&open)){ImGui::End();return;}
    if(ImGui::CollapsingHeader("Getting Started",ImGuiTreeNodeFlags_DefaultOpen)){
        ImGui::TextWrapped("MAWI Studio is an early Linux editor for creating 3D worlds. Start with New Project, choose a name and an existing destination folder, then Create Project. Use Open Project to select a folder containing project.meta. Import Wavefront OBJ geometry from Model > Import OBJ after opening a project.");
    }
    if(ImGui::CollapsingHeader("Interface Overview")){
        ImGui::TextWrapped("Viewport: view and select scene objects. Explorer: scene hierarchy and selection. Properties: edit position, rotation, scale and color. Toolbar: Select, Move, Rotate and Scale modes. Console: status and errors; close its tab to hide it, then use Show console to reopen.");
    }
    if(ImGui::CollapsingHeader("Working with Objects")){
        ImGui::TextWrapped("Click an object in the viewport or Explorer to select it. Select mode only selects. In Move mode, drag to change X/Y position. Rotate changes Y rotation; Scale changes X/Y scale. Properties edits all transform axes and pivot. Modeling includes duplicate, delete and face extrude; choose a face index and distance. Right drag orbits; the wheel zooms.");
    }
    if(ImGui::CollapsingHeader("Projects and Files")){
        ImGui::TextWrapped("Projects live in your chosen folder with project.meta, scenes/main.mawi and assets/. Save writes the scene in MAWI_PROJECT 2 format; version 1 scenes still open. Model > Import OBJ adds geometry without textures; Export Selected OBJ writes the selected object. .mawi is not a Roblox format. MDL, RBX, RBXM and Roblox maps are not supported. Recent projects are stored in the Linux user configuration directory.");
    }
    if(ImGui::CollapsingHeader("Controls and Shortcuts")){
        ImGui::BulletText("Ctrl+S: save the open scene");ImGui::BulletText("Ctrl+O: open the project chooser");
        ImGui::BulletText("Esc: quit MAWI Studio");ImGui::BulletText("Right mouse drag in viewport: orbit camera");
        ImGui::BulletText("Mouse wheel in viewport: zoom");
    }
    if(ImGui::CollapsingHeader("Troubleshooting")){
        ImGui::TextWrapped("Invalid project? Check that its folder contains project.meta and scenes/main.mawi. Missing or malformed files are reported without replacing the scene. OBJ imports must contain vertices and faces; unsupported extensions show an error. Check write permissions when saving fails. X11 and compatible OpenGL are required.");
    }
    if(ImGui::CollapsingHeader("About MAWI Studio")){
        ImGui::TextUnformatted("MAWI Studio 0.1.0");ImGui::TextUnformatted("Developed by: P. Albardeiro");
        ImGui::TextWrapped("Goal: create 3D worlds and projects on Linux, with future interoperability work. This early editor supports geometry-only OBJ exchange; game runtime, MDL and Roblox file compatibility are planned, not available.");
    }
    ImGui::End();
}

void draw_home(HomeState& state){
    ImGui::SetNextWindowPos(ImVec2(0,0));ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Welcome to MAWI Studio",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoDocking);
    ImGui::SetCursorPos(ImVec2(34,32));ImGui::BeginChild("Welcome actions",ImVec2(ImGui::GetContentRegionAvail().x*.55f,0),false);
    draw_mawi_mark(ImGui::GetCursorScreenPos(),44);ImGui::Dummy(ImVec2(50,44));ImGui::SameLine();
    ImGui::BeginGroup();ImGui::TextUnformatted("MAWI Studio");ImGui::TextDisabled("Build worlds and 3D projects on Linux.");ImGui::EndGroup();
    ImGui::Spacing();ImGui::Separator();ImGui::Spacing();
    if(ImGui::Button(u8"\uf067  New Project",ImVec2(220,38))){state.error.clear();state.newDialog=true;}
    if(ImGui::Button(u8"\uf07c  Open Project",ImVec2(220,38))){state.error.clear();state.openDialog=true;}
    if(ImGui::Button(u8"\uf1b2  Import 3D Model",ImVec2(220,38)))state.importPrompt=true;
    if(ImGui::IsItemHovered())ImGui::SetTooltip("Wavefront OBJ; open or create a project first");
    ImGui::Spacing();if(ImGui::Button(u8"\uf02d  Help & Documentation"))state.help=true;
    if(ImGui::Button(u8"\uf013  Settings"))state.settings=true;
    ImGui::TextDisabled("Version 0.1.0");ImGui::TextDisabled("Developed by: P. Albardeiro");
    ImGui::EndChild();ImGui::SameLine();
    ImGui::BeginChild("Recent projects",ImVec2(0,0),true);
    ImGui::TextUnformatted("Recent Projects");ImGui::Separator();
    if(!state.error.empty()&&!state.newDialog&&!state.openDialog)ImGui::TextWrapped("%s",state.error.c_str());
    if(state.recent.empty()){ImGui::TextWrapped("No recent projects yet. Create your first project to get started.");if(ImGui::Button("Create first project"))state.newDialog=true;}
    else {for(size_t i=0;i<state.recent.size();++i){const auto& p=state.recent[i];
        ImGui::PushID(static_cast<int>(i));if(i==0)ImGui::TextDisabled("Last Opened Project");
        ImGui::TextUnformatted(p.name.c_str());ImGui::TextWrapped("%s",p.directory.string().c_str());
        if(!p.lastOpened.empty())ImGui::TextDisabled("Last opened: %s",p.lastOpened.c_str());
        if(!p.created.empty())ImGui::TextDisabled("Created: %s",p.created.c_str());
        if(ImGui::Button("Open project")){state.chosenPath=p.directory.string();state.action=HomeAction::Open;}
        ImGui::SameLine();if(ImGui::Button("Remove from list")){std::string error;
            if(forget_project(p.directory,error))state.recent=read_recent();else state.error=error;
            ImGui::PopID();break;}
        ImGui::Separator();ImGui::PopID();}}
    ImGui::EndChild();ImGui::End();
    if(state.newDialog)ImGui::OpenPopup("New Project");
    if(ImGui::BeginPopupModal("New Project",&state.newDialog,ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::InputText("Project name",state.name,sizeof(state.name));
        ImGui::InputText("Destination folder",state.parent,sizeof(state.parent));ImGui::SameLine();
        if(ImGui::Button("Choose folder")){auto path=choose_folder();if(!path.empty())copy(state.parent,sizeof(state.parent),path);}
        if(!state.error.empty())ImGui::TextWrapped("%s",state.error.c_str());
        if(ImGui::Button("Create Project")){state.action=HomeAction::Create;}
        ImGui::SameLine();if(ImGui::Button("Cancel")){state.newDialog=false;ImGui::CloseCurrentPopup();}
        ImGui::EndPopup();}
    if(state.openDialog)ImGui::OpenPopup("Open Project");
    if(ImGui::BeginPopupModal("Open Project",&state.openDialog,ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::InputText("Project folder",state.openPath,sizeof(state.openPath));ImGui::SameLine();
        if(ImGui::Button("Choose folder")){auto path=choose_folder();if(!path.empty())copy(state.openPath,sizeof(state.openPath),path);}
        if(!state.error.empty())ImGui::TextWrapped("%s",state.error.c_str());
        if(ImGui::Button("Open")){state.chosenPath=state.openPath;state.action=HomeAction::Open;}
        ImGui::SameLine();if(ImGui::Button("Cancel")){state.openDialog=false;ImGui::CloseCurrentPopup();}
        ImGui::EndPopup();}
    if(state.importPrompt)ImGui::OpenPopup("Import Model");
    if(ImGui::BeginPopupModal("Import Model",&state.importPrompt,ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::TextWrapped("Open or create a project, then use Model > Import OBJ in the editor.\nSupported format: Wavefront .obj (mesh geometry only). MDL and Roblox files are not supported yet.");
        if(ImGui::Button("New Project")){state.importPrompt=false;state.newDialog=true;ImGui::CloseCurrentPopup();}
        ImGui::SameLine();if(ImGui::Button("Open Project")){state.importPrompt=false;state.openDialog=true;ImGui::CloseCurrentPopup();}
        ImGui::SameLine();if(ImGui::Button("Close")){state.importPrompt=false;ImGui::CloseCurrentPopup();}
        ImGui::EndPopup();}
}
