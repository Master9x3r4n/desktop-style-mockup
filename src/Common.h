#pragma once
#include "imgui.h"

inline constexpr float kScale = 1.4f;
inline constexpr float kFontPx = 22.0f;

inline float px(float v) { return v * kScale; }
inline float TaskbarH() { return px(58.0f); }

enum class TaskbarAlignment {
    Left = 0,
    Center = 1
};

struct Desktop {
    bool showFiles = false;
    bool showSysInfo = false;
    bool showTaskMgr = false;
    bool showSettings = false;
    bool startOpen = false;
    bool startJustOpened = false;
    bool startButtonHovered = false;
    bool shutdownRequested = false;

    // Settings
    TaskbarAlignment taskbarAlign = TaskbarAlignment::Center;
    int accentColorIdx = 0; // 0: Sky Blue, 1: Violet, 2: Emerald, 3: Coral
};

inline ImVec4 GetAccentColor(int idx) {
    switch (idx) {
        case 1:  return ImVec4(0.60f, 0.40f, 0.98f, 1.0f); // Violet
        case 2:  return ImVec4(0.14f, 0.78f, 0.55f, 1.0f); // Emerald
        case 3:  return ImVec4(0.98f, 0.45f, 0.28f, 1.0f); // Coral
        default: return ImVec4(0.18f, 0.58f, 0.98f, 1.0f); // Fluent Blue
    }
}

inline ImU32 GetAccentColorU32(int idx, float alpha = 1.0f) {
    ImVec4 c = GetAccentColor(idx);
    c.w = alpha;
    return ImGui::ColorConvertFloat4ToU32(c);
}