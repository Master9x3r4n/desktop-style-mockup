#include "TaskManager.h"
#include "Common.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstring>

struct Process {
    const char* name;
    float baseCpu;
    int   baseMemMB;
};

static const Process kProcesses[] = {
    {"System",           2.0f,  180},
    {"CSOPESY Shell",    5.5f,  320},
    {"Compositor",       8.0f,  410},
    {"Scheduler",        1.5f,   64},
    {"Memory Manager",   1.0f,   96},
    {"File Service",     0.8f,   48},
    {"Network Service",  0.5f,   72},
    {"Audio Service",    0.3f,   35},
    {"Input Driver",     0.2f,   18},
};
static const int kProcessCount = sizeof(kProcesses) / sizeof(kProcesses[0]);

static float DummyCpu(int i, double t) {
    float v = kProcesses[i].baseCpu * (1.0f + 0.25f * (float)std::sin(t * 1.3 + i * 1.7));
    return v < 0.0f ? 0.0f : v;
}

static int DummyMem(int i, double t) {
    return kProcesses[i].baseMemMB + (int)(6.0 * std::sin(t * 0.7 + i));
}

void DrawTaskManager(bool& open) {
    ImGui::SetNextWindowSize(ImVec2(px(620), px(460)), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(px(280), px(40)), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Task Manager", &open)) { ImGui::End(); return; }

    double t = glfwGetTime();

    float totalCpu = 0.0f;
    int totalMem = 0;
    for (int i = 0; i < kProcessCount; i++) {
        totalCpu += DummyCpu(i, t);
        totalMem += DummyMem(i, t);
    }
    if (totalCpu > 100.0f) totalCpu = 100.0f;

    static float cpuHist[90] = {};
    static double lastSample = 0.0;
    if (t - lastSample > 0.25) {
        std::memmove(cpuHist, cpuHist + 1, sizeof(cpuHist) - sizeof(float));
        cpuHist[89] = totalCpu;
        lastSample = t;
    }

    if (ImGui::BeginTabBar("tm_tabs")) {
        if (ImGui::BeginTabItem("Processes")) {
            ImGui::Text("CPU: %.1f%%    Memory: %d MB / 8192 MB", totalCpu, totalMem);
            ImGui::Spacing();

            ImGuiTableFlags tf = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
                                 ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
            if (ImGui::BeginTable("procs", 4, tf)) {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, px(60));
                ImGui::TableSetupColumn("CPU", ImGuiTableColumnFlags_WidthFixed, px(90));
                ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthFixed, px(100));
                ImGui::TableHeadersRow();

                for (int i = 0; i < kProcessCount; i++) {
                    float cpu = DummyCpu(i, t);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(kProcesses[i].name);
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", 100 + i * 37);
                    ImGui::TableSetColumnIndex(2);
                    float k = cpu / 10.0f; if (k > 1.0f) k = 1.0f;
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                           ImGui::GetColorU32(ImVec4(0.9f, 0.5f, 0.1f, 0.10f + 0.35f * k)));
                    ImGui::Text("%.1f %%", cpu);
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%d MB", DummyMem(i, t));
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Performance")) {
            ImGui::Text("CPU  %.1f%%", totalCpu);
            ImGui::PlotLines("##cpu", cpuHist, 90, 0, nullptr, 0.0f, 100.0f, ImVec2(-1, px(110)));
            ImGui::Spacing();
            ImGui::Text("Memory  %d MB / 8192 MB", totalMem);
            ImGui::ProgressBar((float)totalMem / 8192.0f, ImVec2(-1, px(26)));
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}