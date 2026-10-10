#include "SettingsApp.h"

void DrawSettingsWindow(Desktop& d) {
    if (!d.showSettings) return;

    ImGui::SetNextWindowSize(ImVec2(px(700), px(480)), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(px(140), px(80)), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Settings", &d.showSettings)) {
        ImGui::End();
        return;
    }

    static int currentTab = 0;
    const char* tabs[] = {"Taskbar", "Personalization", "System", "About"};

    // Sidebar
    ImGui::BeginChild("settings_sidebar", ImVec2(px(180), 0), ImGuiChildFlags_Borders);
    ImGui::Spacing();
    for (int i = 0; i < 4; i++) {
        bool selected = (currentTab == i);
        if (ImGui::Selectable(tabs[i], selected, 0, ImVec2(0, px(32)))) {
            currentTab = i;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // content area
    ImGui::BeginChild("settings_content", ImVec2(0, 0), true);
    
    // Taskbar Settings Tab
    if (currentTab == 0) {
        ImGui::SetWindowFontScale(1.15f);
        ImGui::Text("Taskbar Behaviors & Alignment");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Taskbar Alignment:");
        int align = (d.taskbarAlign == TaskbarAlignment::Center) ? 1 : 0;
        if (ImGui::RadioButton("Left", &align, 0)) {
            d.taskbarAlign = TaskbarAlignment::Left;
        }
        ImGui::SameLine(px(120));
        if (ImGui::RadioButton("Center", &align, 1)) {
            d.taskbarAlign = TaskbarAlignment::Center;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextDisabled("Other Taskbar Settings (Mock)");
        static bool autoHide = false;
        static bool showBadges = true;
        static bool flashIcons = true;
        ImGui::Checkbox("Automatically hide the taskbar in desktop mode", &autoHide);
        ImGui::Checkbox("Show notification badges on taskbar apps", &showBadges);
        ImGui::Checkbox("Flash taskbar buttons when apps require attention", &flashIcons);
    }
    // Personalization Tab
    else if (currentTab == 1) {
        ImGui::SetWindowFontScale(1.15f);
        ImGui::Text("Personalization");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Separator();
        ImGui::Spacing();

        // wallpaper selectiuon
        ImGui::Text("Desktop Wallpaper:");
        const char* wpNames[] = { "Classic", "Grass", "Magic", "Slop" };
        for (int i = 0; i < 4; i++) {
            if (ImGui::RadioButton(wpNames[i], d.wallpaperPreset == i)) {
                d.wallpaperPreset = i;
            }
            if (i < 3) ImGui::SameLine(px(100) * (i + 1));
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // accent color
        ImGui::Text("Accent Color:");
        const char* colors[] = {"Sussy Blue", "Ok Porple", "Cutest Green", "Gabby Orange"};
        for (int i = 0; i < 4; i++) {
            ImVec4 col = GetAccentColor(i);
            ImGui::PushStyleColor(ImGuiCol_Button, col);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, col);
            ImGui::PushID(i);
            if (ImGui::Button("##swatch", ImVec2(px(32), px(32)))) {
                d.accentColorIdx = i;
            }
            ImGui::PopID();
            ImGui::PopStyleColor(3);

            if (d.accentColorIdx == i) {
                ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
                ImGui::GetWindowDrawList()->AddRect(a, b, IM_COL32(255, 255, 255, 255), px(4.0f), 0, 2.0f);
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();
        ImGui::Text("Selected Accent: %s", colors[d.accentColorIdx]);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextDisabled("Themes & Effects (Mock)");
        static bool darkTheme = true;
        static bool transparency = true;
        ImGui::Checkbox("Dark Mode", &darkTheme);
        ImGui::Checkbox("Enable Transparency", &transparency);
    }
    // System Tab
    else if (currentTab == 2) {
        ImGui::SetWindowFontScale(1.15f);
        ImGui::Text("System Display & Audio");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Separator();
        ImGui::Spacing();

        static float brightness = 80.0f;
        ImGui::SliderFloat("Display Brightness", &brightness, 0.0f, 100.0f, "%.0f%%");

        static bool nightLight = false;
        ImGui::Checkbox("Night Light", &nightLight);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::SliderFloat("Master Volume", &d.masterVolume, 0.0f, 100.0f, "%.0f%%");

        static int powerMode = 1;
        const char* powerModes[] = {"Power Saver", "Balanced", "Best Performance"};
        ImGui::Combo("Power Mode", &powerMode, powerModes, 3);
    }
    // about tab
    else if (currentTab == 3) {
        ImGui::SetWindowFontScale(1.15f);
        ImGui::Text("About CSOPESY OS");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("OS Edition     : CSOPESY Desktop Shell (Preview)");
        ImGui::Text("Version        : 24H2");
        ImGui::Text("Framework      : Dear ImGui 1.91 + OpenGL 3.3");
        ImGui::Text("System Type    : 64-bit operating system, x64-based processor");
        ImGui::Spacing();
        if (ImGui::Button("Check for Updates", ImVec2(px(160), px(32)))) {
            ImGui::OpenPopup("UpdateNotice");
        }
        if (ImGui::BeginPopup("UpdateNotice")) {
            ImGui::Text("Your system is up to date.");
            ImGui::EndPopup();
        }
    }

    ImGui::EndChild();
    ImGui::End();
}