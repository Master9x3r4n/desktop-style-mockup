#pragma once

#include <imgui.h>
#include <GLFW/glfw3.h>

// Function declaration for rendering an icon tile with image texture + fallback badge
void DrawIcon(ImDrawList* dl, ImVec2 iMin, ImVec2 iMax, float iconSz, const char* label, ImU32 color, const char* badge);