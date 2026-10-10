#include "StartMenu.h"
#include "IconRenderer.h"
#include <cfloat>

static bool DrawPinnedApp(const char* title, const char* subtitle, ImU32 tileCol, const char* badge) {
    ImGui::PushID(title);
    float rowW = ImGui::GetContentRegionAvail().x;
    float rowH = px(44.0f);
    ImVec2 p = ImGui::GetCursorScreenPos();

    bool clicked = ImGui::Selectable("##row", false, 0, ImVec2(rowW, rowH));
    bool hovered = ImGui::IsItemHovered();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pMax(p.x + rowW, p.y + rowH);

    if (hovered) {
        dl->AddRectFilled(p, pMax, IM_COL32(255, 255, 255, 18), px(6.0f));
    }

    float iconSz = px(32.0f);
    ImVec2 iMin(p.x + px(6), p.y + (rowH - iconSz) * 0.5f);
    ImVec2 iMax(iMin.x + iconSz, iMin.y + iconSz);
    
    DrawIcon(dl, iMin, iMax, iconSz, title, tileCol, badge);

    float tx = iMax.x + px(12.0f);
    dl->AddText(ImVec2(tx, p.y + px(4.0f)), IM_COL32(240, 244, 252, 255), title);
    dl->AddText(ImVec2(tx, p.y + px(4.0f) + ImGui::GetFontSize() + px(1.0f)),
                IM_COL32(165, 172, 185, 200), subtitle);

    ImGui::PopID();
    return clicked;
}

static void DocumentRow(const char* name, const char* timeAgo) {
    ImGui::PushID(name);
    float rowW = ImGui::GetContentRegionAvail().x;
    float rowH = px(30.0f);
    ImVec2 p = ImGui::GetCursorScreenPos();

    ImGui::Selectable("##rec_item", false, 0, ImVec2(rowW, rowH));
    bool hovered = ImGui::IsItemHovered();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pMax(p.x + rowW, p.y + rowH);

    if (hovered) {
        dl->AddRectFilled(p, pMax, IM_COL32(255, 255, 255, 14), px(5.0f));
    }

    float tx = p.x + px(8.0f);
    float ty = p.y + (rowH - ImGui::GetFontSize()) * 0.5f;
    dl->AddText(ImVec2(tx, ty), IM_COL32(230, 235, 245, 255), name);

    ImVec2 timeSz = ImGui::CalcTextSize(timeAgo);
    dl->AddText(ImVec2(pMax.x - timeSz.x - px(8.0f), ty),
                IM_COL32(145, 150, 160, 180), timeAgo);

    ImGui::PopID();
}

void DrawStartMenu(Desktop& d) {
    if (!d.startOpen) return;

    ImVec2 disp = ImGui::GetIO().DisplaySize;
    float w = px(430.0f);
    float h = px(580.0f);

    float posX = px(12.0f);
    if (d.taskbarAlign == TaskbarAlignment::Center) {
        posX = (disp.x - w) * 0.5f;
    }
    float posY = disp.y - TaskbarH() - h - px(10.0f);

    ImGui::SetNextWindowPos(ImVec2(posX, posY));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::SetNextWindowBgAlpha(0.96f);
    if (d.startJustOpened) {
        ImGui::SetNextWindowFocus();
        d.startJustOpened = false;
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, px(14.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(16), px(14)));
    ImGui::Begin("##startmenu", nullptr, flags);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 winMin = ImGui::GetWindowPos();
    ImVec2 winMax = ImVec2(winMin.x + w, winMin.y + h);

    dl->AddRect(winMin, winMax, IM_COL32(255, 255, 255, 28), px(14.0f), 0, 1.2f);

    // Top Search Box
    static char searchBuf[64] = "";
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, px(8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(px(10), px(8)));
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##startsearch", "Search apps, settings, files...", searchBuf, sizeof(searchBuf));
    ImGui::PopStyleVar(2);

    ImGui::Spacing();

    // pinned apps
    ImGui::TextDisabled("Pinned Apps");
    ImGui::Spacing();
    ImGui::BeginChild("pinned_scroll", ImVec2(0, px(240)), false);
    {
        if (DrawPinnedApp("File Explorer", "Browse files and directories", IM_COL32(230, 160, 40, 255), "EXP")) {
            d.showFiles = true; 
            d.startOpen = false; 
            ImGui::SetWindowFocus("File Explorer");
        }
        if (DrawPinnedApp("System Info", "Hardware & OS specs", IM_COL32(50, 140, 240, 255), "SYS")) {
            d.showSysInfo = true; 
            d.startOpen = false; 
            ImGui::SetWindowFocus("System Info");
        }
        if (DrawPinnedApp("Task Manager", "Processes and resource activity", IM_COL32(40, 180, 115, 255), "TSK")) {
            d.showTaskMgr = true; 
            d.startOpen = false; 
            ImGui::SetWindowFocus("Task Manager");
        }
        if (DrawPinnedApp("Settings", "Personalization & preferences", IM_COL32(110, 125, 145, 255), "SET")) {
            d.showSettings = true; 
            d.startOpen = false; 
            ImGui::SetWindowFocus("Settings");
        }
        if (DrawPinnedApp("Wuhdle", "Wuhdleeee", IM_COL32(83, 141, 78, 255), "WUH")) {
            d.showWordle = true; 
            d.startOpen = false; 
            ImGui::SetWindowFocus("Wuhdle");
        }
        if (DrawPinnedApp("ChicaGPT", "Find out whats inside your head", IM_COL32(210, 80, 160, 255), "GPT")) {
            d.showAI = true; 
            d.startOpen = false; 
            ImGui::SetWindowFocus("ChicaGPT");
        }
        if (DrawPinnedApp("Calculator", "For doing math", IM_COL32(210, 110, 50, 255), "CAL")) {
            d.showCalc = true; 
            d.startOpen = false; 
            ImGui::SetWindowFocus("Calculator");
        }
        if (DrawPinnedApp("Reels", "When Sas3000 is boring", IM_COL32(230, 45, 85, 255), "REL")) {
            d.showReels = true; 
            d.startOpen = false; 
            ImGui::SetWindowFocus("Reels");
        }
        if (DrawPinnedApp("Terminal", "Command prompt & shell", IM_COL32(40, 42, 54, 255), "TRM")) {
            d.showTerminal = true;
            d.startOpen = false;
            ImGui::SetWindowFocus("Terminal");
        }
    }
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Recent Documents (mock data)
    ImGui::TextDisabled("Recent Documents");
    ImGui::Spacing();
    DocumentRow("scheduler.cpp", "25m ago");
    DocumentRow("output2.pptx", "2h ago");
    DocumentRow("notes.txt", "Yesterday");

    // User and Power (footer area)
    float footerH = px(52.0f);
    float footerY = h - footerH;
    ImGui::SetCursorPosY(footerY - px(8.0f));
    ImGui::Separator();

    ImVec2 fPos = ImGui::GetCursorScreenPos();
    float cy = fPos.y + footerH * 0.5f;

    float r = px(15.0f);
    float cx = fPos.x + r + px(4.0f);
    dl->AddCircleFilled(ImVec2(cx, cy), r, IM_COL32(60, 110, 190, 255), 24);
    dl->AddCircleFilled(ImVec2(cx, cy - r * 0.2f), r * 0.35f, IM_COL32(255, 255, 255, 235), 16);
    dl->AddCircleFilled(ImVec2(cx, cy + r * 0.75f), r * 0.65f, IM_COL32(255, 255, 255, 235), 16);
    dl->AddCircleFilled(ImVec2(cx + r * 0.7f, cy + r * 0.7f), px(3.5f), IM_COL32(50, 210, 100, 255), 12);

    float userTx = cx + r + px(10.0f);
    dl->AddText(ImVec2(userTx, cy - ImGui::GetFontSize() + px(1.0f)), IM_COL32(240, 245, 255, 255), "Guest");
    dl->AddText(ImVec2(userTx, cy + px(1.0f)), IM_COL32(150, 155, 165, 200), "Standard User");

    float pwrW = px(64.0f);
    float pwrH = px(34.0f);
    ImGui::SetCursorPosX(w - pwrW - ImGui::GetStyle().WindowPadding.x);
    ImGui::SetCursorPosY(footerY + (footerH - pwrH) * 0.5f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, px(6.0f));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.72f, 0.16f, 0.16f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.88f, 0.22f, 0.22f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
    if (ImGui::Button("PWR", ImVec2(pwrW, pwrH))) {
        d.shutdownRequested = true;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows |
                                          ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    if ((ImGui::IsMouseClicked(0) && !hovered && !d.startButtonHovered) ||
        ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        d.startOpen = false;
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
}