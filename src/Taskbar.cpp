#include "Taskbar.h"
#include <ctime>
#include <cstdio>
#include <cfloat>

static bool TaskbarAppButton(const char* id, const char* label, bool isOpen, int accentIdx) {
    ImVec2 btnSize(px(58), px(44));
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, px(8.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, isOpen 
        ? ImVec4(0.24f, 0.27f, 0.36f, 0.90f) 
        : ImVec4(0.15f, 0.17f, 0.23f, 0.40f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.32f, 0.44f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.19f, 0.22f, 0.30f, 1.0f));

    bool clicked = ImGui::Button(label, btnSize);

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRect(min, max, IM_COL32(255, 255, 255, isOpen ? 35 : 15), px(8.0f));

    if (isOpen) {
        float indW = px(20.0f);
        float indH = px(3.5f);
        float cx = (min.x + max.x) * 0.5f;
        dl->AddRectFilled(
            ImVec2(cx - indW * 0.5f, max.y - indH - px(2)),
            ImVec2(cx + indW * 0.5f, max.y - px(2)),
            GetAccentColorU32(accentIdx),
            px(2.0f)
        );
    }

    return clicked;
}

static bool StartMenuButton(Desktop& d) {
    ImVec2 btnSize(px(50), px(44));

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, px(8.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, d.startOpen 
        ? ImVec4(0.26f, 0.30f, 0.42f, 0.95f) 
        : ImVec4(0.15f, 0.17f, 0.23f, 0.50f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.35f, 0.48f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.18f, 0.22f, 0.32f, 1.0f));

    bool clicked = ImGui::Button("##start_btn", btnSize);
    d.startButtonHovered = ImGui::IsItemHovered();

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRect(min, max, IM_COL32(255, 255, 255, d.startOpen ? 45 : 20), px(8.0f));

    float sz = px(8.5f);
    float gap = px(3.0f);
    float cx = (min.x + max.x) * 0.5f;
    float cy = (min.y + max.y) * 0.5f;

    ImVec2 tl(cx - sz - gap * 0.5f, cy - sz - gap * 0.5f);
    ImVec2 tr(cx + gap * 0.5f,      cy - sz - gap * 0.5f);
    ImVec2 bl(cx - sz - gap * 0.5f, cy + gap * 0.5f);
    ImVec2 br(cx + gap * 0.5f,      cy + gap * 0.5f);

    float r = px(2.5f);
    dl->AddRectFilled(tl, ImVec2(tl.x + sz, tl.y + sz), IM_COL32(56, 189, 248, 255), r);
    dl->AddRectFilled(tr, ImVec2(tr.x + sz, tr.y + sz), IM_COL32(37, 99, 235, 255),  r);
    dl->AddRectFilled(bl, ImVec2(bl.x + sz, bl.y + sz), IM_COL32(2, 132, 199, 255),  r);
    dl->AddRectFilled(br, ImVec2(br.x + sz, br.y + sz), IM_COL32(99, 102, 241, 255), r);

    return clicked;
}

static void DrawSystemTray(Desktop& d, float barWidth, float barHeight) {
    std::time_t now = std::time(nullptr);
    std::tm* lt = std::localtime(&now);
    char timeBuf[16], dateBuf[24];
    std::strftime(timeBuf, sizeof(timeBuf), "%I:%M %p", lt);
    std::strftime(dateBuf, sizeof(dateBuf), "%m/%d/%Y", lt);

    float volW = px(56);
    float clockW = px(120);
    float pwrW = px(52);
    float elemH = px(44);
    float spacing = px(8);
    float marginR = px(12);

    float totalTrayW = volW + spacing + clockW + spacing + pwrW;
    float startX = barWidth - totalTrayW - marginR;
    float startY = (barHeight - elemH) * 0.5f;

    // 1. VOL Button
    ImGui::SetCursorPos(ImVec2(startX, startY));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, px(8.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.20f, 0.28f, 0.70f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.28f, 0.38f, 0.90f));
    if (ImGui::Button("VOL", ImVec2(volW, elemH))) {
        ImGui::OpenPopup("VolumeFlyout");
    }
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();

    // Volume Card Popup
    float flyoutW = px(260.0f);
    float flyoutX = startX + (volW - flyoutW) * 0.5f;
    if (flyoutX + flyoutW > barWidth - marginR) {
        flyoutX = barWidth - marginR - flyoutW;
    }
    if (flyoutX < px(12.0f)) flyoutX = px(12.0f);

    ImGui::SetNextWindowPos(ImVec2(flyoutX, ImGui::GetIO().DisplaySize.y - barHeight - px(10.0f)),
                            ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    ImGui::SetNextWindowSize(ImVec2(flyoutW, 0.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, px(12.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(16), px(12)));
    if (ImGui::BeginPopup("VolumeFlyout", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {
        ImGui::Text("Volume");
        ImGui::SameLine(flyoutW - px(80));
        if (d.masterVolume <= 0.001f) {
            ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "Muted");
        } else {
            ImGui::TextColored(GetAccentColor(d.accentColorIdx), "%.0f%%", d.masterVolume);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::SliderFloat("##vol_slider", &d.masterVolume, 0.0f, 100.0f, "%.0f%%");

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);

    // 2. Clock & Date Pill
    float clockX = startX + volW + spacing;
    ImGui::SetCursorPos(ImVec2(clockX, startY));
    ImGui::InvisibleButton("##clock_tray", ImVec2(clockW, elemH));
    bool clockHovered = ImGui::IsItemHovered();

    ImVec2 cMin = ImGui::GetItemRectMin();
    ImVec2 cMax = ImGui::GetItemRectMax();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(cMin, cMax, clockHovered ? IM_COL32(255, 255, 255, 22) : IM_COL32(255, 255, 255, 10), px(8.0f));
    dl->AddRect(cMin, cMax, IM_COL32(255, 255, 255, 18), px(8.0f));

    ImVec2 szTime = ImGui::CalcTextSize(timeBuf);
    ImVec2 szDate = ImGui::CalcTextSize(dateBuf);
    float textGap = px(1.0f);
    float totalTextH = szTime.y + szDate.y + textGap;
    float textTopY = cMin.y + (elemH - totalTextH) * 0.5f;

    dl->AddText(ImVec2(cMin.x + (clockW - szTime.x) * 0.5f, textTopY),
                IM_COL32(255, 255, 255, 245), timeBuf);
    dl->AddText(ImVec2(cMin.x + (clockW - szDate.x) * 0.5f, textTopY + szTime.y + textGap),
                IM_COL32(190, 195, 205, 200), dateBuf);

    // 3. Tray PWR Button
    float pwrX = clockX + clockW + spacing;
    ImGui::SetCursorPos(ImVec2(pwrX, startY));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, px(8.0f));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.72f, 0.16f, 0.16f, 0.75f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.88f, 0.22f, 0.22f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
    if (ImGui::Button("PWR", ImVec2(pwrW, elemH))) {
        d.shutdownRequested = true;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
}

void DrawTaskbar(Desktop& d) {
    ImVec2 disp = ImGui::GetIO().DisplaySize;
    float barH = TaskbarH();

    ImGui::SetNextWindowPos(ImVec2(0, disp.y - barH));
    ImGui::SetNextWindowSize(ImVec2(disp.x, barH));
    ImGui::SetNextWindowBgAlpha(0.88f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##taskbar", nullptr, flags);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddLine(
        ImVec2(0, disp.y - barH),
        ImVec2(disp.x, disp.y - barH),
        IM_COL32(255, 255, 255, 24),
        1.0f
    );

    float elemH = px(44);
    float elemY = (barH - elemH) * 0.5f;

    float startW = px(50);
    float appW = px(58);
    float spacing = px(6);

    // 4 apps on taskbar: SET (Settings), FIL (Files), SYS (System), TSK (Task Manager)
    int appCount = 4;
    float groupWidth = startW + spacing + (appCount * appW) + ((appCount - 1) * spacing);

    float startX = px(12.0f);
    if (d.taskbarAlign == TaskbarAlignment::Center) {
        startX = (disp.x - groupWidth) * 0.5f;
        if (startX < px(12.0f)) startX = px(12.0f);
    }

    // 1. Start Button
    ImGui::SetCursorPos(ImVec2(startX, elemY));
    if (StartMenuButton(d)) {
        d.startOpen = !d.startOpen;
        d.startJustOpened = d.startOpen;
    }

    // 2. Apps in order: Settings, File, System, Task Manager
    float curX = startX + startW + spacing;

    // SET (Settings)
    ImGui::SetCursorPos(ImVec2(curX, elemY));
    if (TaskbarAppButton("btn_sett", "SET", d.showSettings, d.accentColorIdx))
        d.showSettings = !d.showSettings;
    curX += appW + spacing;

    // FIL (Files)
    ImGui::SetCursorPos(ImVec2(curX, elemY));
    if (TaskbarAppButton("btn_files", "FIL", d.showFiles, d.accentColorIdx))
        d.showFiles = !d.showFiles;
    curX += appW + spacing;

    // SYS (System)
    ImGui::SetCursorPos(ImVec2(curX, elemY));
    if (TaskbarAppButton("btn_sys", "SYS", d.showSysInfo, d.accentColorIdx))
        d.showSysInfo = !d.showSysInfo;
    curX += appW + spacing;

    // TSK (Task Manager)
    ImGui::SetCursorPos(ImVec2(curX, elemY));
    if (TaskbarAppButton("btn_task", "TSK", d.showTaskMgr, d.accentColorIdx))
        d.showTaskMgr = !d.showTaskMgr;

    // 3. System Tray (VOL, Clock, PWR)
    DrawSystemTray(d, disp.x, barH);

    ImGui::End();
    ImGui::PopStyleVar(2);
}