#include "DesktopOverlay.h"
#include <ctime>

void DrawClockAndPower(Desktop& d) {
    ImVec2 size = ImGui::GetIO().DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(size.x - px(12.0f), px(12.0f)), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.45f);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, px(10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(14), px(10)));
    ImGui::Begin("##clock_power", nullptr, flags);

    std::time_t now = std::time(nullptr);
    std::tm* lt = std::localtime(&now);
    char timeBuf[32], dateBuf[48];
    std::strftime(timeBuf, sizeof(timeBuf), "%I:%M:%S %p", lt);
    std::strftime(dateBuf, sizeof(dateBuf), "%a, %b %d %Y", lt);

    ImGui::BeginGroup();
    ImGui::SetWindowFontScale(1.4f);
    ImGui::TextUnformatted(timeBuf);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::TextDisabled("%s", dateBuf);
    ImGui::EndGroup();

    ImGui::SameLine(0.0f, px(16));

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.10f, 0.10f, 1.0f));
    if (ImGui::Button("PWR", ImVec2(px(60), px(52))))
        d.shutdownRequested = true;
    ImGui::PopStyleColor(3);

    ImGui::End();
    ImGui::PopStyleVar(2);
}