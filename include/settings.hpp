#pragma once
#include <filesystem>
#include <string>

struct Settings {
    int theme = 0;             // 0 graphite, 1 light, 2 high contrast
    float orbitSensitivity = 0.45f;
    float zoomSensitivity = 0.5f;
    bool invertOrbitY = false;
    bool showGrid = true;
    bool showAxes = true;
    int fpsLimit = 30;         // 0 = unlimited
    float renderScale = 1.f;
    int windowWidth = 1180;
    int windowHeight = 760;
};

enum class SettingsPreset { Balanced, Performance, Quality };
void apply_preset(Settings& settings, SettingsPreset preset);
std::filesystem::path settings_file();
Settings load_settings();
bool save_settings(const Settings& settings, std::string& error);
void draw_settings(Settings& settings, bool& open, bool& applyWindowSize);
void apply_theme(int theme);
