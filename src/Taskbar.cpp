#include "Taskbar.h"
#include <ctime>
#include <cstdio>

// Compact modern taskbar button with bottom indicator bar
static bool TaskbarAppButton(const char* id, const char* label, bool isOpen, int accentIdx) {
    ImVec2 btnSize(px(64), px(44));
    
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

    // Subtle outline
    dl->AddRect(min, max, IM_COL32(255, 255, 255, isOpen ? 35 : 15), px(8.0f));

    // Active bottom pill indicator
    if (isOpen) {
        float indW = px(22.0f);
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

// Reimagined modern start logo button
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

    // Modern 4-quadrant gradient logo inside start button
    float sz = px(8.5f);
    float gap = px(3.0f);
    float cx = (min.x + max.x) * 0.5f;
    float cy = (min.y + max.y) * 0.5f;

    ImVec2 tl(cx - sz - gap * 0.5f, cy - sz - gap * 0.5f);
    ImVec2 tr(cx + gap * 0.5f,      cy - sz - gap * 0.5f);
    ImVec2 bl(cx - sz - gap * 0.5f, cy + gap * 0.5f);
    ImVec2 br(cx + gap * 0.5f,      cy + gap * 0.5f);

    float r = px(2.5f);
    dl->AddRectFilled(tl, ImVec2(tl.x + sz, tl.y + sz), IM_COL32(56, 189, 248, 255), r);  // Sky Cyan
    dl->AddRectFilled(tr, ImVec2(tr.x + sz, tr.y + sz), IM_COL32(37, 99, 235, 255),  r);  // Royal Blue
    dl->AddRectFilled(bl, ImVec2(bl.x + sz, bl.y + sz), IM_COL32(2, 132, 199, 255),  r);  // Deep Blue
    dl->AddRectFilled(br, ImVec2(br.x + sz, br.y + sz), IM_COL32(99, 102, 241, 255), r);  // Indigo

    return clicked;
}

// Pixel-perfect System Tray (Clock, Date, PWR)
static void DrawSystemTray(Desktop& d, float barWidth, float barHeight) {
    std::time_t now = std::time(nullptr);
    std::tm* lt = std::localtime(&now);
    char timeBuf[16], dateBuf[24];
    std::strftime(timeBuf, sizeof(timeBuf), "%I:%M %p", lt);
    std::strftime(dateBuf, sizeof(dateBuf), "%m/%d/%Y", lt);

    float pwrW = px(52);
    float clockW = px(120);
    float elemH = px(44);
    float spacing = px(8);
    float marginR = px(12);

    float totalTrayW = clockW + spacing + pwrW;
    float startX = barWidth - totalTrayW - marginR;
    float startY = (barHeight - elemH) * 0.5f;

    // 1. Clock & Date Pill Widget
    ImGui::SetCursorPos(ImVec2(startX, startY));
    ImGui::InvisibleButton("##clock_tray", ImVec2(clockW, elemH));
    bool clockHovered = ImGui::IsItemHovered();

    ImVec2 cMin = ImGui::GetItemRectMin();
    ImVec2 cMax = ImGui::GetItemRectMax();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Background pill for clock
    dl->AddRectFilled(cMin, cMax, clockHovered ? IM_COL32(255, 255, 255, 22) : IM_COL32(255, 255, 255, 10), px(8.0f));
    dl->AddRect(cMin, cMax, IM_COL32(255, 255, 255, 18), px(8.0f));

    // Centered vertical layout for Time and Date
    ImVec2 szTime = ImGui::CalcTextSize(timeBuf);
    ImVec2 szDate = ImGui::CalcTextSize(dateBuf);
    float textGap = px(1.0f);
    float totalTextH = szTime.y + szDate.y + textGap;
    float textTopY = cMin.y + (elemH - totalTextH) * 0.5f;

    dl->AddText(ImVec2(cMin.x + (clockW - szTime.x) * 0.5f, textTopY),
                IM_COL32(255, 255, 255, 245), timeBuf);
    dl->AddText(ImVec2(cMin.x + (clockW - szDate.x) * 0.5f, textTopY + szTime.y + textGap),
                IM_COL32(190, 195, 205, 200), dateBuf);

    // 2. Tray PWR Button
    ImGui::SetCursorPos(ImVec2(startX + clockW + spacing, startY));
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

    // Top acrylic line
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddLine(
        ImVec2(0, disp.y - barH),
        ImVec2(disp.x, disp.y - barH),
        IM_COL32(255, 255, 255, 24),
        1.0f
    );

    // Layout configuration
    float elemH = px(44);
    float elemY = (barH - elemH) * 0.5f;

    float startW = px(50);
    float appW = px(64);
    float spacing = px(8);
    int appCount = 4; // FILE, SYS, TASK, SETT
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

    // 2. Taskbar App Buttons
    float curX = startX + startW + spacing;

    ImGui::SetCursorPos(ImVec2(curX, elemY));
    if (TaskbarAppButton("btn_files", "FILE", d.showFiles, d.accentColorIdx))
        d.showFiles = !d.showFiles;
    curX += appW + spacing;

    ImGui::SetCursorPos(ImVec2(curX, elemY));
    if (TaskbarAppButton("btn_sys", "SYS", d.showSysInfo, d.accentColorIdx))
        d.showSysInfo = !d.showSysInfo;
    curX += appW + spacing;

    ImGui::SetCursorPos(ImVec2(curX, elemY));
    if (TaskbarAppButton("btn_task", "TASK", d.showTaskMgr, d.accentColorIdx))
        d.showTaskMgr = !d.showTaskMgr;
    curX += appW + spacing;

    ImGui::SetCursorPos(ImVec2(curX, elemY));
    if (TaskbarAppButton("btn_sett", "SETT", d.showSettings, d.accentColorIdx))
        d.showSettings = !d.showSettings;

    // 3. System Tray (Clock & PWR)
    DrawSystemTray(d, disp.x, barH);

    ImGui::End();
    ImGui::PopStyleVar(2);
}