#include "settings.hpp"
#include "workspace.hpp"
#include "imgui.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace fs=std::filesystem;

void apply_preset(Settings& s,SettingsPreset p){
    if(p==SettingsPreset::Performance){s.fpsLimit=30;s.renderScale=.5f;s.showGrid=false;s.showAxes=true;}
    else if(p==SettingsPreset::Quality){s.fpsLimit=60;s.renderScale=1.f;s.showGrid=true;s.showAxes=true;}
    else{s.fpsLimit=30;s.renderScale=.75f;s.showGrid=true;s.showAxes=true;}
}

fs::path settings_file(){auto recent=recent_file();return recent.empty()?fs::path{}:recent.parent_path()/"settings.conf";}

Settings load_settings(){
    Settings s;auto path=settings_file();if(path.empty())return s;
    std::ifstream in(path);std::string key;double value;
    while(in>>key>>value){
        if(key=="theme")s.theme=std::clamp(int(value),0,2);
        else if(key=="orbit_sensitivity")s.orbitSensitivity=std::clamp(float(value),.05f,2.f);
        else if(key=="zoom_sensitivity")s.zoomSensitivity=std::clamp(float(value),.05f,5.f);
        else if(key=="invert_orbit_y")s.invertOrbitY=value!=0;
        else if(key=="show_grid")s.showGrid=value!=0;
        else if(key=="show_axes")s.showAxes=value!=0;
        else if(key=="fps_limit")s.fpsLimit=std::clamp(int(value),0,120);
        else if(key=="render_scale")s.renderScale=std::clamp(float(value),.5f,1.f);
        else if(key=="window_width")s.windowWidth=std::clamp(int(value),800,3840);
        else if(key=="window_height")s.windowHeight=std::clamp(int(value),560,2160);
    }
    return s;
}

bool save_settings(const Settings& s,std::string& error){
    auto path=settings_file();if(path.empty()){error="Cannot locate the Linux config directory";return false;}
    std::error_code ec;fs::create_directories(path.parent_path(),ec);
    if(ec){error="Cannot create config directory: "+ec.message();return false;}
    auto temp=path;temp+=".tmp";
    std::ofstream out(temp,std::ios::trunc);if(!out){error="Cannot write settings";return false;}
    out<<"theme "<<s.theme<<"\norbit_sensitivity "<<s.orbitSensitivity<<"\nzoom_sensitivity "<<s.zoomSensitivity
       <<"\ninvert_orbit_y "<<s.invertOrbitY<<"\nshow_grid "<<s.showGrid<<"\nshow_axes "<<s.showAxes
       <<"\nfps_limit "<<s.fpsLimit<<"\nrender_scale "<<s.renderScale
       <<"\nwindow_width "<<s.windowWidth<<"\nwindow_height "<<s.windowHeight<<'\n';
    out.close();if(!out){error="Cannot finish settings file";return false;}
    fs::rename(temp,path,ec);if(ec){error="Cannot replace settings file: "+ec.message();return false;}
    return true;
}

void draw_settings(Settings& s,bool& open,bool& applyWindowSize){
    if(!open)return;
    ImGui::SetNextWindowSize(ImVec2(610,580),ImGuiCond_FirstUseEver);
    if(!ImGui::Begin("Settings",&open)){ImGui::End();return;}
    if(ImGui::BeginTabBar("Settings categories")){
        if(ImGui::BeginTabItem("Appearance")){
            const char* themes[]={"Graphite","Light","High contrast"};
            ImGui::Combo("Theme",&s.theme,themes,3);
            ImGui::TextDisabled("Theme applies immediately and is saved for the next launch.");
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Camera & Input")){
            ImGui::SliderFloat("Orbit sensitivity",&s.orbitSensitivity,.05f,2.f,"%.2f");
            ImGui::SliderFloat("Zoom sensitivity",&s.zoomSensitivity,.05f,5.f,"%.2f");
            ImGui::Checkbox("Invert vertical orbit",&s.invertOrbitY);
            ImGui::TextDisabled("Right mouse drag orbits; the wheel zooms in the viewport.");
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Viewport & Performance")){
            if(ImGui::Button("Performance preset"))apply_preset(s,SettingsPreset::Performance);
            ImGui::SameLine();if(ImGui::Button("Balanced preset"))apply_preset(s,SettingsPreset::Balanced);
            ImGui::SameLine();if(ImGui::Button("Quality preset"))apply_preset(s,SettingsPreset::Quality);
            ImGui::Separator();
            ImGui::Checkbox("Show grid",&s.showGrid);ImGui::Checkbox("Show axes",&s.showAxes);
            const char* fps[]={"Unlimited","30 FPS","60 FPS","120 FPS"};
            int choice=s.fpsLimit==0?0:s.fpsLimit==30?1:s.fpsLimit==60?2:3;
            if(ImGui::Combo("Frame limit",&choice,fps,4))s.fpsLimit=choice==0?0:choice==1?30:choice==2?60:120;
            const char* scales[]={"50%","75%","100%"};int scale=s.renderScale<.625f?0:s.renderScale<.875f?1:2;
            if(ImGui::Combo("3D render scale",&scale,scales,3))s.renderScale=scale==0?.5f:scale==1?.75f:1.f;
            ImGui::TextWrapped("Render scale affects the 3D scene texture only. The editor interface remains at native resolution.");
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Window")){
            const char* resolutions[]={"1024 x 768","1280 x 720","1366 x 768","1600 x 900","1920 x 1080"};
            const int widths[]={1024,1280,1366,1600,1920},heights[]={768,720,768,900,1080};
            int current=-1;for(int i=0;i<5;++i)if(widths[i]==s.windowWidth&&heights[i]==s.windowHeight)current=i;
            if(ImGui::BeginCombo("Window size",current<0?"Custom":resolutions[current])){
                for(int i=0;i<5;++i)if(ImGui::Selectable(resolutions[i],current==i)){s.windowWidth=widths[i];s.windowHeight=heights[i];applyWindowSize=true;}
                ImGui::EndCombo();}
            ImGui::InputInt("Width",&s.windowWidth);ImGui::InputInt("Height",&s.windowHeight);
            if(ImGui::Button("Apply window size")){s.windowWidth=std::clamp(s.windowWidth,800,3840);s.windowHeight=std::clamp(s.windowHeight,560,2160);applyWindowSize=true;}
            ImGui::TextDisabled("Changes the application window; desktop resolution is unchanged.");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::Separator();
    if(ImGui::Button("Restore defaults")){s=Settings{};applyWindowSize=true;}
    ImGui::SameLine();ImGui::TextDisabled("Changes are saved automatically.");
    ImGui::End();
}

void apply_theme(int value){
    auto& s=ImGui::GetStyle();
    if(value==1){ImGui::StyleColorsLight();return;}
    if(value==2){s.Colors[ImGuiCol_WindowBg]=ImVec4(.07f,.07f,.07f,1);s.Colors[ImGuiCol_Text]=ImVec4(1,1,1,1);s.Colors[ImGuiCol_Button]=ImVec4(.32f,.32f,.32f,1);return;}
    // The graphite palette is supplied by the existing editor theme.
}
