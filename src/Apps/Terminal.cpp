#include "Terminal.h"
#include <vector>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <algorithm>

struct TerminalState {
    char inputBuf[256] = "";
    std::vector<std::string> items;
    bool scrollToBottom = false;
    bool initialized = false;

    void AddLog(const char* fmt, ...) {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        items.push_back(std::string(buf));
        scrollToBottom = true;
    }

    void ClearLog() {
        items.clear();
    }

    void ExecCommand(const char* commandLine, Desktop& d, bool& open) {
        AddLog("> %s", commandLine);

        std::string cmd(commandLine);
        if (cmd.empty()) return;

        if (cmd == "clear" || cmd == "cls") {
            ClearLog();
        }
        else if (cmd == "help") {
            AddLog("Commands:");
            AddLog("  help             - Display this help menu");
            AddLog("  clear            - Clear terminal screen");
            AddLog("  sysinfo          - Display OS details");
            AddLog("  vol <0-100>      - View or set master volume");
            AddLog("  echo <text>      - Print text to screen");
            AddLog("  exit             - Close terminal window");
        }
        else if (cmd == "sysinfo" || cmd == "neofetch") {
            AddLog("OS: Custom ImGui Desktop Environment");
            AddLog("Host: C++ ImGui Application");
            AddLog("Status: Running");
        }
        else if (cmd.rfind("vol", 0) == 0) {
            if (cmd.length() > 4) {
                try {
                    float val = std::stof(cmd.substr(4));
                    d.masterVolume = std::clamp(val, 0.0f, 100.0f);
                    AddLog("Master volume set to %.0f%%", d.masterVolume);
                }
                catch (...) {
                    AddLog("Usage: vol <0-100>");
                }
            }
            else {
                AddLog("Current volume: %.0f%%", d.masterVolume);
            }
        }
        else if (cmd.rfind("echo ", 0) == 0) {
            AddLog("%s", cmd.substr(5).c_str());
        }
        else if (cmd == "exit") {
            open = false;
        }
        else {
            AddLog("Unknown command: '%s'. Type 'help' for commands.", commandLine);
        }

        scrollToBottom = true;
    }
};

static TerminalState gTerminal;

void DrawTerminalWindow(bool& open, Desktop& d) {
    if (!open) return;

    if (!gTerminal.initialized) {
        gTerminal.AddLog("OS Terminal v1.0.0");
        gTerminal.AddLog("Type 'help' for a list of available commands.");
        gTerminal.AddLog("-------------------------------------------");
        gTerminal.initialized = true;
    }

    ImGui::SetNextWindowSize(ImVec2(px(520.0f), px(320.0f)), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Terminal", &open)) {
        ImGui::End();
        return;
    }

    if (ImGui::SmallButton("Clear")) { gTerminal.ClearLog(); }
    ImGui::SameLine();
    bool copyToClipboard = ImGui::SmallButton("Copy Log");
    ImGui::Separator();

    const float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeightToReserve), false, ImGuiWindowFlags_HorizontalScrollbar);

    if (copyToClipboard) {
        ImGui::LogToClipboard();
    }

    for (const auto& item : gTerminal.items) {
        if (item.rfind(">", 0) == 0) {
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", item.c_str());
        }
        else if (item.rfind("Unknown", 0) == 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", item.c_str());
        }
        else {
            ImGui::TextUnformatted(item.c_str());
        }
    }

    if (copyToClipboard) {
        ImGui::LogFinish();
    }

    if (gTerminal.scrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    gTerminal.scrollToBottom = false;

    ImGui::EndChild();
    ImGui::Separator();

    ImGui::Text(">");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.0f);

    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
    if (ImGui::InputText("##CommandInput", gTerminal.inputBuf, IM_ARRAYSIZE(gTerminal.inputBuf), inputFlags)) {
        gTerminal.ExecCommand(gTerminal.inputBuf, d, open);
        gTerminal.inputBuf[0] = '\0';
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();
}