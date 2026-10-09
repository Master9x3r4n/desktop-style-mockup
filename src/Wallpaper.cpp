#include "Wallpaper.h"
#include "Common.h"
#include "imgui.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#include <cstdint>
#include <string>

struct CachedTexture {
    GLuint tex = 0;
    int w = 0;
    int h = 0;
    bool attempted = false;
};

// Index 0: Grass (wallpaper1.png), 1: Magic (wallpaper2.png), 2: Slop (wallpaper3.png)
static CachedTexture gWallTex[3];

static GLuint LoadTexture(const char* path, int& outW, int& outH) {
    int channels = 0;
    unsigned char* data = stbi_load(path, &outW, &outH, &channels, 4);
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

static void DrawProceduralWallpaper(ImDrawList* dl, ImVec2 size) {
    ImU32 topL = IM_COL32(16, 22, 48, 255);
    ImU32 topR = IM_COL32(40, 28, 84, 255);
    ImU32 botR = IM_COL32(88, 40, 110, 255);
    ImU32 botL = IM_COL32(24, 60, 100, 255);
    dl->AddRectFilledMultiColor(ImVec2(0, 0), size, topL, topR, botR, botL);

    dl->AddCircleFilled(ImVec2(size.x * 0.78f, size.y * 0.30f), size.y * 0.35f, IM_COL32(255, 255, 255, 10), 96);
    dl->AddCircleFilled(ImVec2(size.x * 0.20f, size.y * 0.75f), size.y * 0.28f, IM_COL32(255, 255, 255, 8), 96);

    const float step = 40.0f;
    for (float y = step * 0.5f; y < size.y; y += step) {
        for (float x = step * 0.5f; x < size.x; x += step) {
            dl->AddCircleFilled(ImVec2(x, y), 1.5f, IM_COL32(255, 255, 255, 28));
        }
    }
}

void DrawWallpaper(const Desktop& d) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 size = ImGui::GetIO().DisplaySize;

    // Preset 0: Classic (Default layout if no image)
    if (d.wallpaperPreset == 0) {
        DrawProceduralWallpaper(dl, size);
        return;
    }

    // Presets 1: Grass, 2: Magic, 3: Slop
    int imgIdx = d.wallpaperPreset - 1;
    if (imgIdx >= 0 && imgIdx < 3) {
        CachedTexture& item = gWallTex[imgIdx];
        if (!item.attempted) {
            item.attempted = true;
            const char* fileNames[] = { "wallpaper1.png", "wallpaper2.png", "wallpaper3.png" };
            std::string path1 = std::string(PROJECT_DIR) + "/wallpaper/" + fileNames[imgIdx];
            std::string path2 = std::string("wallpaper/") + fileNames[imgIdx];

            item.tex = LoadTexture(path1.c_str(), item.w, item.h);
            if (!item.tex) {
                item.tex = LoadTexture(path2.c_str(), item.w, item.h);
            }
        }

        if (item.tex) {
            float imgAspect = (float)item.w / (float)item.h;
            float scrAspect = size.x / size.y;
            ImVec2 uv0(0, 0), uv1(1, 1);
            if (scrAspect > imgAspect) {
                float visible = imgAspect / scrAspect;
                uv0.y = (1.0f - visible) * 0.5f;
                uv1.y = 1.0f - uv0.y;
            } else {
                float visible = scrAspect / imgAspect;
                uv0.x = (1.0f - visible) * 0.5f;
                uv1.x = 1.0f - uv0.x;
            }
            dl->AddImage((ImTextureID)(intptr_t)item.tex, ImVec2(0, 0), size, uv0, uv1);
            return;
        }
    }

    // Fallback if image failed to load
    DrawProceduralWallpaper(dl, size);
}

static void DesktopIcon(const char* label, const char* badge, ImU32 color, bool& appOpen) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    float itemW = px(84.0f);
    float itemH = px(92.0f);

    ImGui::PushID(label);
    bool clicked = ImGui::Selectable("##icon", false, 0, ImVec2(itemW, itemH));
    bool hovered = ImGui::IsItemHovered();

    if (clicked || (hovered && ImGui::IsMouseDoubleClicked(0))) {
        appOpen = true;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pMax(p.x + itemW, p.y + itemH);

    if (hovered) {
        dl->AddRectFilled(p, pMax, IM_COL32(255, 255, 255, 28), px(8.0f));
        dl->AddRect(p, pMax, IM_COL32(255, 255, 255, 45), px(8.0f), 0, 1.0f);
    }

    float iconSz = px(46.0f);
    float ix = p.x + (itemW - iconSz) * 0.5f;
    float iy = p.y + px(8.0f);
    ImVec2 iMin(ix, iy);
    ImVec2 iMax(ix + iconSz, iy + iconSz);

    dl->AddRectFilled(ImVec2(iMin.x, iMin.y + px(2)), ImVec2(iMax.x, iMax.y + px(2)), IM_COL32(0, 0, 0, 70), px(10.0f));
    dl->AddRectFilled(iMin, iMax, color, px(10.0f));
    dl->AddRect(iMin, iMax, IM_COL32(255, 255, 255, 40), px(10.0f));

    ImVec2 bSz = ImGui::CalcTextSize(badge);
    dl->AddText(ImVec2(iMin.x + (iconSz - bSz.x) * 0.5f, iMin.y + (iconSz - bSz.y) * 0.5f),
                IM_COL32(255, 255, 255, 255), badge);

    ImVec2 lSz = ImGui::CalcTextSize(label);
    float tx = p.x + (itemW - lSz.x) * 0.5f;
    float ty = iy + iconSz + px(5.0f);
    dl->AddText(ImVec2(tx + 1, ty + 1), IM_COL32(0, 0, 0, 220), label);
    dl->AddText(ImVec2(tx, ty), IM_COL32(255, 255, 255, 255), label);

    ImGui::PopID();
}

void DrawDesktopIcons(Desktop& d) {
    ImGui::SetNextWindowPos(ImVec2(px(20), px(20)), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

    ImGui::Begin("##desktop_icons", nullptr, flags);

    DesktopIcon("Wuhdle", "WUH", IM_COL32(83, 141, 78, 255), d.showWordle);
    ImGui::Spacing();
    DesktopIcon("ChicaGPT", "GPT", IM_COL32(210, 80, 160, 255), d.showAI);
    ImGui::Spacing();
    DesktopIcon("Calculator", "CAL", IM_COL32(210, 110, 50, 255), d.showCalc);

    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}