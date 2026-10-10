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

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cstdio>

static void LoadFonts(ImGuiIO& io, float sizePx);
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

    // helper functions for fonts and theme
    LoadFonts(io, kFontPx);
    ApplyOSTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glfwSetWindowCloseCallback(window, [](GLFWwindow* w) {
        glfwSetWindowShouldClose(w, GLFW_FALSE);
    });

    Desktop desktop;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame(); // start drawing

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

        // Taskbar & Start Menu
        DrawTaskbar(desktop);
        DrawStartMenu(desktop);

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

static void LoadFonts(ImGuiIO& io, float sizePx) {
    const char* paths[] = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
    };
    for (const char* p : paths) {
        if (FILE* f = std::fopen(p, "rb")) {
            std::fclose(f);
            if (io.Fonts->AddFontFromFileTTF(p, sizePx)) return;
        }
    }
    ImFontConfig cfg;
    cfg.SizePixels = sizePx;
    io.Fonts->AddFontDefault(&cfg);
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