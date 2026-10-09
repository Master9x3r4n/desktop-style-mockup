#include "SystemInfo.h"
#include "Common.h"
#include <GLFW/glfw3.h>

void DrawSysInfoWindow(bool& open) {
    ImGui::SetNextWindowSize(ImVec2(px(420), px(300)), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(px(160), px(110)), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("System Info", &open)) { ImGui::End(); return; }

    ImGui::SetWindowFontScale(1.3f);
    ImGui::Text("CSOPESY OS");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::TextDisabled("Version 1.0 (mock-up build)");
    ImGui::Separator();

    int up = (int)glfwGetTime();
    ImGui::Text("Kernel      : csopesy-sim 0.1");
    ImGui::Text("Processor   : Virtual CPU @ 3.0 GHz (4 cores)");
    ImGui::Text("Memory      : 8192 MB");
    ImGui::Text("Display     : %.0f x %.0f", ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
    ImGui::Text("Uptime      : %02d:%02d:%02d", up / 3600, (up / 60) % 60, up % 60);
    ImGui::Text("Frame rate  : %.1f FPS", ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::TextWrapped("This is placeholder information for the System Info screen.");
    ImGui::End();
}