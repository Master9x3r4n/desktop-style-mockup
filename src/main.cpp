#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F   // missing from Windows' old gl.h
#endif

// =====================================================================
//  CSOPESY Desktop-Style OS Mock-up  (GLFW + OpenGL3 + Dear ImGui)
//  Layers, drawn back to front every frame:
//    1. Desktop   : wallpaper (background draw list) + clock + PWR button
//    2. Apps      : Files, System Info, Task Manager windows
//    3. Taskbar   : fixed bottom panel with 3 icon buttons
// =====================================================================

// ---------------------------------------------------------------------
//  State
// ---------------------------------------------------------------------
struct Desktop {
    bool showFiles = false;
    bool showSysInfo = false;
    bool showTaskMgr = false;
    bool shutdownRequested = false;
};

struct Process {
    const char* name;
    float baseCpu;   // percent (dummy)
    int   baseMemMB; // megabytes (dummy)
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

static const float kTaskbarHeight = 52.0f;

// Dummy "live" values so the Task Manager looks alive (still fake data).
static float DummyCpu(int i, double t) {
    float v = kProcesses[i].baseCpu * (1.0f + 0.25f * (float)std::sin(t * 1.3 + i * 1.7));
    return v < 0.0f ? 0.0f : v;
}
static int DummyMem(int i, double t) {
    return kProcesses[i].baseMemMB + (int)(6.0 * std::sin(t * 0.7 + i));
}

// ---------------------------------------------------------------------
//  Layer 1: Desktop (wallpaper, clock, PWR)
// ---------------------------------------------------------------------
// Loads an image file into an OpenGL texture. Returns 0 on failure.
static GLuint LoadTexture(const char* path, int& outW, int& outH) {
    int channels = 0;
    unsigned char* data = stbi_load(path, &outW, &outH, &channels, 4); // force RGBA
    if (!data) return 0;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outW, outH, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return tex;
}

static void DrawWallpaper() {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 size = ImGui::GetIO().DisplaySize;

    // Try to load the image once, on the first call
    static bool tried = false;
    static GLuint wallTex = 0;
    static int wallW = 0, wallH = 0;
    if (!tried) {
        tried = true;
        const char* candidates[] = {
            PROJECT_DIR "/wallpaper/wallpaper.png",
            PROJECT_DIR "/wallpaperfolder/wallpaper.png",
            "wallpaper/wallpaper.png",
            "wallpaperfolder/wallpaper.png",
        };
        for (const char* p : candidates) {
            wallTex = LoadTexture(p, wallW, wallH);
            if (wallTex) break;
        }
    }

    // Image found: draw it "cover" style (fills the window, crops the excess)
    if (wallTex) {
        float imgAspect = (float)wallW / (float)wallH;
        float scrAspect = size.x / size.y;
        ImVec2 uv0(0, 0), uv1(1, 1);
        if (scrAspect > imgAspect) {            // window is wider: crop top/bottom
            float visible = imgAspect / scrAspect;
            uv0.y = (1.0f - visible) * 0.5f;
            uv1.y = 1.0f - uv0.y;
        } else {                                // window is taller: crop left/right
            float visible = scrAspect / imgAspect;
            uv0.x = (1.0f - visible) * 0.5f;
            uv1.x = 1.0f - uv0.x;
        }
        dl->AddImage((ImTextureID)(intptr_t)wallTex, ImVec2(0, 0), size, uv0, uv1);
        return;
    }

    // ---- Fallback: your original procedural wallpaper ----
    ImU32 topL = IM_COL32(16, 22, 48, 255);
    ImU32 topR = IM_COL32(40, 28, 84, 255);
    ImU32 botR = IM_COL32(88, 40, 110, 255);
    ImU32 botL = IM_COL32(24, 60, 100, 255);
    dl->AddRectFilledMultiColor(ImVec2(0, 0), size, topL, topR, botR, botL);

    dl->AddCircleFilled(ImVec2(size.x * 0.78f, size.y * 0.30f), size.y * 0.35f, IM_COL32(255, 255, 255, 10), 96);
    dl->AddCircleFilled(ImVec2(size.x * 0.20f, size.y * 0.75f), size.y * 0.28f, IM_COL32(255, 255, 255, 8), 96);

    const float step = 40.0f;
    for (float y = step * 0.5f; y < size.y; y += step)
        for (float x = step * 0.5f; x < size.x; x += step)
            dl->AddCircleFilled(ImVec2(x, y), 1.5f, IM_COL32(255, 255, 255, 28));
}

static void DrawClockAndPower(Desktop& d) {
    ImVec2 size = ImGui::GetIO().DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(size.x - 12.0f, 12.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.45f);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 10));
    ImGui::Begin("##clock_power", nullptr, flags);

    // Real-time clock, re-read every frame
    std::time_t now = std::time(nullptr);
    std::tm* lt = std::localtime(&now);
    char timeBuf[32], dateBuf[48];
    std::strftime(timeBuf, sizeof(timeBuf), "%I:%M:%S %p", lt);
    std::strftime(dateBuf, sizeof(dateBuf), "%a, %b %d %Y", lt);

    ImGui::BeginGroup();
    ImGui::SetWindowFontScale(1.5f);
    ImGui::TextUnformatted(timeBuf);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::TextDisabled("%s", dateBuf);
    ImGui::EndGroup();

    ImGui::SameLine(0.0f, 16.0f);

    // PWR button = the ONLY way to close the app
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.10f, 0.10f, 1.0f));
    if (ImGui::Button("PWR", ImVec2(56, 44)))
        d.shutdownRequested = true;
    ImGui::PopStyleColor(3);

    ImGui::End();
    ImGui::PopStyleVar(2);
}

// ---------------------------------------------------------------------
//  Layer 2: Apps
// ---------------------------------------------------------------------
static void DrawFilesWindow(bool& open) {
    ImGui::SetNextWindowSize(ImVec2(520, 320), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(80, 80), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Files", &open)) { ImGui::End(); return; }

    ImGui::TextDisabled("C:\\Users\\Guest\\Documents");
    ImGui::Separator();

    struct Row { const char* name; const char* type; const char* size; const char* modified; };
    static const Row rows[] = {
        {"Reports",         "Folder",   "--",      "2026-09-28"},
        {"Pictures",        "Folder",   "--",      "2026-09-20"},
        {"notes.txt",       "Text",     "2 KB",    "2026-10-01"},
        {"scheduler.cpp",   "C++ File", "14 KB",   "2026-10-03"},
        {"wallpaper.png",   "Image",    "1.2 MB",  "2026-09-15"},
        {"output2.pptx",    "Slides",   "3.8 MB",  "2026-10-05"},
    };

    if (ImGui::BeginTable("files", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Size");
        ImGui::TableSetupColumn("Modified");
        ImGui::TableHeadersRow();
        for (const Row& r : rows) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Selectable(r.name, false, ImGuiSelectableFlags_SpanAllColumns);
            ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(r.type);
            ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(r.size);
            ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(r.modified);
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

static void DrawSysInfoWindow(bool& open) {
    ImGui::SetNextWindowSize(ImVec2(380, 260), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(220, 140), ImGuiCond_FirstUseEver);
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

static void DrawTaskManager(bool& open) {
    ImGui::SetNextWindowSize(ImVec2(560, 420), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(360, 60), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Task Manager", &open)) { ImGui::End(); return; }

    double t = glfwGetTime();

    // Totals
    float totalCpu = 0.0f;
    int totalMem = 0;
    for (int i = 0; i < kProcessCount; i++) { totalCpu += DummyCpu(i, t); totalMem += DummyMem(i, t); }
    if (totalCpu > 100.0f) totalCpu = 100.0f;

    // CPU history for the Performance tab (sampled 4x per second)
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
                ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("CPU", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableHeadersRow();

                for (int i = 0; i < kProcessCount; i++) {
                    float cpu = DummyCpu(i, t);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(kProcesses[i].name);
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", 100 + i * 37);
                    ImGui::TableSetColumnIndex(2);
                    // Heat-map cell colour: more CPU = more orange
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
            ImGui::PlotLines("##cpu", cpuHist, 90, 0, nullptr, 0.0f, 100.0f, ImVec2(-1, 100));
            ImGui::Spacing();
            ImGui::Text("Memory  %d MB / 8192 MB", totalMem);
            ImGui::ProgressBar((float)totalMem / 8192.0f, ImVec2(-1, 22));
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

// ---------------------------------------------------------------------
//  Layer 3: Taskbar
// ---------------------------------------------------------------------
// Button that shows a highlight + underline while its app is open.
static bool TaskbarButton(const char* label, bool isOpen) {
    if (isOpen) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.30f, 0.45f, 0.75f, 1.0f));
    bool clicked = ImGui::Button(label, ImVec2(120, 36));
    if (isOpen) ImGui::PopStyleColor();

    if (isOpen) {
        ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(a.x + 14, b.y + 3), ImVec2(b.x - 14, b.y + 5),
                                                  IM_COL32(120, 190, 255, 255), 2.0f);
    }
    return clicked;
}

static void DrawTaskbar(Desktop& d) {
    ImVec2 size = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0, size.y - kTaskbarHeight));
    ImGui::SetNextWindowSize(ImVec2(size.x, kTaskbarHeight));
    ImGui::SetNextWindowBgAlpha(0.85f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::Begin("##taskbar", nullptr, flags);

    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(ImVec4(0.55f, 0.75f, 1.0f, 1.0f), "CSOPESY");
    ImGui::SameLine(0.0f, 20.0f);

    if (TaskbarButton("Files", d.showFiles))          d.showFiles = !d.showFiles;
    ImGui::SameLine();
    if (TaskbarButton("System Info", d.showSysInfo))  d.showSysInfo = !d.showSysInfo;
    ImGui::SameLine();
    if (TaskbarButton("Task Manager", d.showTaskMgr)) d.showTaskMgr = !d.showTaskMgr;

    ImGui::End();
    ImGui::PopStyleVar(3);
}

// ---------------------------------------------------------------------
//  Main
// ---------------------------------------------------------------------
int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "CSOPESY OS", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // don't persist window layout between runs
    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowRounding = 8.0f;
    ImGui::GetStyle().FrameRounding = 4.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Block the window's X button / Alt+F4: the PWR button is the only way out.
    glfwSetWindowCloseCallback(window, [](GLFWwindow* w) {
        glfwSetWindowShouldClose(w, GLFW_FALSE);
    });

    Desktop desktop;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Desktop layer (first thing drawn each frame)
        DrawWallpaper();
        DrawClockAndPower(desktop);

        // 2. Apps
        if (desktop.showFiles)   DrawFilesWindow(desktop.showFiles);
        if (desktop.showSysInfo) DrawSysInfoWindow(desktop.showSysInfo);
        if (desktop.showTaskMgr) DrawTaskManager(desktop.showTaskMgr);

        // 3. Taskbar
        DrawTaskbar(desktop);

        // PWR pressed -> shut down cleanly
        if (desktop.shutdownRequested)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}