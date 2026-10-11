#include "Common.h"
#include "Wallpaper.h"
#include "Taskbar.h"
#include "StartMenu.h"
#include "Apps/FileExplorer.h"
#include "Apps/SystemInfo.h"
#include "Apps/TaskManager.h"
#include "Apps/SettingsApp.h"
#include "Apps/WordleApp.h"
#include "Apps/AiApp.h"
#include "Apps/CalculatorApp.h"
#include "Apps/ReelsApp.h"
#include "Apps/Terminal.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>
#include <cstring>

enum class OSState {
    Booting,
    Running
};

struct BiosLine {
    double delay;       // Delay in seconds before this line appears
    const char* text;   // Content string
    ImVec4 color;       // Text color
};

static void LoadFonts(ImGuiIO& io, float sizePx, OSState OSState);
static void ApplyOSTheme();


int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1600, 900, "CSOPESY OS", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Boot screen and sequence data
    OSState osState = OSState::Booting;
    double startTime = glfwGetTime();
    double totalBootTime = 5.0; // Duration in seconds before showing the desktop

    ImVec4 c_White = ImVec4(0.90f, 0.90f, 0.90f, 1.0f);
    ImVec4 c_Gray = ImVec4(0.55f, 0.55f, 0.55f, 1.0f);

    std::vector<BiosLine> biosSequence = {
        { 0.0, "CSOPESY Megatrends", c_White },
        { 0.0, "Released: 10/09/2026", c_White },
        { 0.0, "Group 4 - CSOPESY S04 (C) 2026 CSOPESY Megatrends, Inc.", c_White },

        { 1.0, "Testing System Memory... 8192MB OK", c_Gray },

        { 1.5, "CPU: Custom Virtual x86 Processor @ 3.0GHz", c_Gray },
        { 1.5, "BIOS Version: CSOPESY Custom BIOS v1.0", c_Gray },

        { 2.0, "Detected Storages...", c_White },
        { 2.0, "  [Target 0] NVMe: CSOPESY-SSD-256GB --> OK (S.M.A.R.T. Healthy)", c_Gray },
        { 2.5, "  [Target 1] USB: Generic Boot Media Found", c_Gray },

        { 3.0, "Booting CSOPESY OS...", c_White }
    };

    // Load initial font and theme
    bool fontSwapped = false;
    LoadFonts(io, kFontPx, osState);
    ApplyOSTheme();

    glfwSetWindowCloseCallback(window, [](GLFWwindow* w) {
        glfwSetWindowShouldClose(w, GLFW_FALSE);
    });

    Desktop desktop;
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (osState == OSState::Running && !fontSwapped) {
            LoadFonts(io, kFontPx, osState);
            fontSwapped = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame(); // start drawing

        double elapsed = glfwGetTime() - startTime;
        if (osState == OSState::Booting) {
            if (elapsed >= totalBootTime) {
                osState = OSState::Running;
            }

            const ImGuiViewport* vp = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(vp->WorkPos);
            ImGui::SetNextWindowSize(vp->WorkSize);

            ImGuiWindowFlags bootFlags = ImGuiWindowFlags_NoDecoration | 
                                         ImGuiWindowFlags_NoMove | 
                                         ImGuiWindowFlags_NoResize | 
                                         ImGuiWindowFlags_NoSavedSettings;

            // Pure black backdrop style context
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(30.0f), px(30.0f)));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, px(4.0f)));

            if (ImGui::Begin("##BiosScreen", nullptr, bootFlags)) {
                // Print each log message when its specified timeline threshold triggers
                for (int i = 0; i < biosSequence.size(); ++i) {
                    if (elapsed >= biosSequence[i].delay) {
                        ImGui::TextColored(biosSequence[i].color, "%s", biosSequence[i].text);
                        
                        if (std::strcmp(biosSequence[i].text, "Group 4 - CSOPESY S04 (C) 2026 CSOPESY Megatrends, Inc.") == 0 ||
                            std::strcmp(biosSequence[i].text, "Testing System Memory... 8192MB OK") == 0 ||
                            std::strcmp(biosSequence[i].text, "BIOS Version: CSOPESY Custom BIOS v1.0") == 0 ||
                            std::strcmp(biosSequence[i].text, "  [Target 1] USB: Generic Boot Media Found") == 0 ||
                            std::strcmp(biosSequence[i].text, "Booting CSOPESY OS...") == 0) {
                            ImGui::Dummy(ImVec2(0.0f, px(20.0f))); // vertical space
                        }
                    }
                }

                // Blinking prompt block
                if (elapsed < totalBootTime) {
                    if (static_cast<int>(elapsed * 2.5) % 2 == 0) {
                        ImGui::Text("_");
                    }
                }
            }
            ImGui::End();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor();
        }
        else if (osState == OSState::Running) {
            // Desktop background & Desktop Icons
            DrawWallpaper(desktop);
            DrawDesktopIcons(desktop);

            //Applications
            if (desktop.showFiles)    DrawFilesWindow(desktop.showFiles);
            if (desktop.showSysInfo)  DrawSysInfoWindow(desktop.showSysInfo);
            if (desktop.showTaskMgr)  DrawTaskManager(desktop.showTaskMgr);
            if (desktop.showSettings) DrawSettingsWindow(desktop);
            if (desktop.showWordle)   DrawWordleWindow(desktop.showWordle);
            if (desktop.showAI)       DrawAiWindow(desktop.showAI);
            if (desktop.showCalc)     DrawCalculatorWindow(desktop.showCalc);
            if (desktop.showReels)    DrawReelsWindow(desktop.showReels);
            if (desktop.showTerminal) DrawTerminalWindow(desktop.showTerminal, desktop);

            // Taskbar & Start Menu
            DrawTaskbar(desktop);
            DrawStartMenu(desktop);
        }

        // PWR button functionality
        if (desktop.shutdownRequested) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        ImGui::Render(); // stop drawing 

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

static void LoadFonts(ImGuiIO& io, float sizePx, OSState osState) {
    io.Fonts->Clear();

    bool fontLoaded = false;
    if (osState == OSState::Booting) {
        if (FILE* f = std::fopen("C:/Windows/Fonts/cour.ttf", "rb")) {
            std::fclose(f);
            if (io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/cour.ttf", sizePx)) {
                fontLoaded = true;
            }
        }
    } else {
        const char* paths[] = {
            "C:/Windows/Fonts/segoeui.ttf",
            "C:/Windows/Fonts/arial.ttf",
        };

        for (const char* p : paths) {
            if (FILE* f = std::fopen(p, "rb")) {
                std::fclose(f);
                if (io.Fonts->AddFontFromFileTTF(p, sizePx)) {
                    fontLoaded = true;
                    break; 
                }
            }
        }
    }

    ImFontConfig cfg;
    cfg.SizePixels = sizePx;
    io.Fonts->AddFontDefault(&cfg);

    ImGui_ImplOpenGL3_DestroyFontsTexture();
    ImGui_ImplOpenGL3_CreateFontsTexture();
}

static void ApplyOSTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(kScale);
    style.WindowRounding    = px(10.0f);
    style.FrameRounding     = px(6.0f);
    style.PopupRounding     = px(10.0f);
    style.ScrollbarRounding = px(10.0f);
    style.GrabRounding      = px(5.0f);
    style.TabRounding       = px(6.0f);
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.PopupBorderSize   = 1.0f;

    // css ahh styling
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]             = ImVec4(0.11f, 0.12f, 0.16f, 0.94f);
    colors[ImGuiCol_PopupBg]              = ImVec4(0.12f, 0.13f, 0.18f, 0.96f);
    colors[ImGuiCol_Border]               = ImVec4(1.00f, 1.00f, 1.00f, 0.09f);
    colors[ImGuiCol_FrameBg]              = ImVec4(0.18f, 0.20f, 0.27f, 0.60f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.24f, 0.26f, 0.35f, 0.80f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.28f, 0.30f, 0.42f, 0.90f);
    colors[ImGuiCol_TitleBg]              = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.14f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_Button]               = ImVec4(0.19f, 0.21f, 0.28f, 0.70f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.26f, 0.29f, 0.39f, 0.90f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.16f, 0.18f, 0.25f, 1.00f);
    colors[ImGuiCol_Header]               = ImVec4(0.22f, 0.24f, 0.33f, 0.65f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.28f, 0.31f, 0.42f, 0.80f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.20f, 0.22f, 0.30f, 1.00f);
    colors[ImGuiCol_Tab]                  = ImVec4(0.15f, 0.16f, 0.22f, 0.80f);
    colors[ImGuiCol_TabHovered]           = ImVec4(0.26f, 0.29f, 0.39f, 0.90f);
    colors[ImGuiCol_TabActive]            = ImVec4(0.22f, 0.25f, 0.34f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.08f, 0.08f, 0.10f, 0.40f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.30f, 0.32f, 0.40f, 0.60f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.42f, 0.52f, 0.80f);
}