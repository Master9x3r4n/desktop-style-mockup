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

void DrawWallpaper() {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 size = ImGui::GetIO().DisplaySize;

    static bool tried = false;
    static GLuint wallTex = 0;
    static int wallW = 0, wallH = 0;
    if (!tried) {
        tried = true;
        const char* candidates[] = {
            PROJECT_DIR "/wallpaper/wallpaper.png",
            "wallpaper/wallpaper.png",
        };
        for (const char* p : candidates) {
            wallTex = LoadTexture(p, wallW, wallH);
            if (wallTex) break;
        }
    }

    if (wallTex) {
        float imgAspect = (float)wallW / (float)wallH;
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
        dl->AddImage((ImTextureID)(intptr_t)wallTex, ImVec2(0, 0), size, uv0, uv1);
        return;
    }

    // Procedural wallpaper fallback
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