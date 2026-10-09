#include "ReelsApp.h"
#include <GLFW/glfw3.h>
#include "stb_image.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>

struct ReelItem {
    GLuint tex = 0;
    int w = 0;
    int h = 0;
    bool attempted = false;
    bool liked = false;
    int likeCount = 1420;
    int commentCount = 84;
    const char* user = "@csopesy";
    const char* caption = "Exploring the desktop shell #os #ui";
};

static ReelItem gReels[5] = {
    {0, 0, 0, false, false, 1420, 84,  "@pieisspy",      "Hoping to find these cats in Genshin"},
    {0, 0, 0, false, false, 2890, 126, "@alittletotheleft",   "Helpful guide for learning insertion sort"},
    {0, 0, 0, false, false, 9540, 432, "@givemeanameidk",      "New chess variant unlocked"},
    {0, 0, 0, false, false, 4120, 95,  "@gdg1106", "I ask my cat how much they know about quantum mechanics"},
    {0, 0, 0, false, false, 6780, 210, "@_pring.les",     "I'm so excited for the Green Yuri anime!"}
};
static int gCurrentReel = 0;

static GLuint LoadReelTexture(const char* path, int& outW, int& outH) {
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
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outW, outH, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return tex;
}

static void EnsureReelLoaded(int idx) {
    if (idx < 0 || idx >= 5) return;
    ReelItem& r = gReels[idx];
    if (r.attempted) return;
    r.attempted = true;

    std::string fileName = "reel" + std::to_string(idx + 1) + ".jpg";
    std::string p1 = std::string(PROJECT_DIR) + "/reels/" + fileName;
    std::string p2 = std::string("reels/") + fileName;
    std::string p3 = std::string("../reels/") + fileName;

    r.tex = LoadReelTexture(p1.c_str(), r.w, r.h);
    if (!r.tex) r.tex = LoadReelTexture(p2.c_str(), r.w, r.h);
    if (!r.tex) r.tex = LoadReelTexture(p3.c_str(), r.w, r.h);

    if (r.tex) {
        std::printf("[Reels] Loaded %s (%dx%d)\n", fileName.c_str(), r.w, r.h);
    }
}

void DrawReelsWindow(bool& open) {
    float winW = px(410.0f);
    float winH = px(670.0f);
    ImGui::SetNextWindowSize(ImVec2(winW, winH), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Reels", &open, ImGuiWindowFlags_NoScrollbar)) {
        ImGui::End();
        return;
    }

    EnsureReelLoaded(gCurrentReel);
    ReelItem& cur = gReels[gCurrentReel];

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 contentP0 = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;
    float availH = ImGui::GetContentRegionAvail().y;

    float actionW = px(84.0f);
    float cardW = availW - actionW - px(12.0f);
    float cardH = availH;
    ImVec2 cardP0 = contentP0;
    ImVec2 cardP1(cardP0.x + cardW, cardP0.y + cardH);

    // 1. Image Viewer (Cover 9:16 aspect ratio)
    if (cur.tex) {
        float imgAspect = (float)cur.w / (float)cur.h;
        float frameAspect = cardW / cardH;
        ImVec2 uv0(0.0f, 0.0f), uv1(1.0f, 1.0f);

        if (frameAspect > imgAspect) {
            float visible = imgAspect / frameAspect;
            uv0.y = (1.0f - visible) * 0.5f;
            uv1.y = 1.0f - uv0.y;
        } else {
            float visible = frameAspect / imgAspect;
            uv0.x = (1.0f - visible) * 0.5f;
            uv1.x = 1.0f - uv0.x;
        }

        dl->AddImage((ImTextureID)(intptr_t)cur.tex, cardP0, cardP1, uv0, uv1);
    } else {
        dl->AddRectFilled(cardP0, cardP1, IM_COL32(24, 26, 36, 255), px(12.0f));
        char buf[64];
        std::snprintf(buf, sizeof(buf), "reel%d.jpg", gCurrentReel + 1);
        ImVec2 tSz = ImGui::CalcTextSize(buf);
        dl->AddText(ImVec2(cardP0.x + (cardW - tSz.x) * 0.5f, cardP0.y + cardH * 0.45f),
                    IM_COL32(200, 205, 220, 220), buf);
        ImVec2 subSz = ImGui::CalcTextSize("Place in reels/ folder");
        dl->AddText(ImVec2(cardP0.x + (cardW - subSz.x) * 0.5f, cardP0.y + cardH * 0.45f + tSz.y + px(6)),
                    IM_COL32(140, 145, 160, 180), "Place in reels/ folder");
    }

    // Border and bottom vignette
    dl->AddRect(cardP0, cardP1, IM_COL32(255, 255, 255, 30), px(10.0f));
    dl->AddRectFilledMultiColor(
        ImVec2(cardP0.x, cardP1.y - px(130.0f)), cardP1,
        IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0),
        IM_COL32(0, 0, 0, 220), IM_COL32(0, 0, 0, 220)
    );

    // Creator info (bottom overlay)
    float bTextY = cardP1.y - px(64.0f);
    dl->AddText(ImVec2(cardP0.x + px(12), bTextY), IM_COL32(255, 255, 255, 255), cur.user);
    dl->AddText(ImVec2(cardP0.x + px(12), bTextY + ImGui::GetFontSize() + px(2)),
                IM_COL32(220, 225, 235, 220), cur.caption);

    // 2. Right Actions: Compact vertical stack with Like & Comment counts retained
    float actionX = cardP1.x + px(10.0f);
    float btnH = px(36.0f);
    float gap = px(6.0f);

    float clusterH = btnH * 5.0f + px(36.0f) + gap * 4.0f;
    float clusterStartY = cardP0.y + (cardH - clusterH) * 0.5f;

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, px(8.0f));

    // A. Up Button (Previous)
    ImGui::SetCursorScreenPos(ImVec2(actionX, clusterStartY));
    if (ImGui::Button("^", ImVec2(actionW, btnH))) {
        gCurrentReel = (gCurrentReel - 1 + 5) % 5;
    }

    // B. Like Button + Like Count
    float curY = clusterStartY + btnH + gap;
    ImGui::SetCursorScreenPos(ImVec2(actionX, curY));
    ImVec4 likeCol = cur.liked ? ImVec4(0.85f, 0.18f, 0.28f, 0.90f) : ImVec4(0.20f, 0.22f, 0.30f, 0.70f);
    ImGui::PushStyleColor(ImGuiCol_Button, likeCol);
    if (ImGui::Button(cur.liked ? "Liked" : "Like", ImVec2(actionW, btnH))) {
        cur.liked = !cur.liked;
        cur.likeCount += cur.liked ? 1 : -1;
    }
    ImGui::PopStyleColor();

    char likeStr[16];
    std::snprintf(likeStr, sizeof(likeStr), "%d", cur.likeCount);
    ImVec2 lSz = ImGui::CalcTextSize(likeStr);
    ImGui::SetCursorScreenPos(ImVec2(actionX + (actionW - lSz.x) * 0.5f, curY + btnH + px(1.0f)));
    ImGui::TextDisabled("%s", likeStr);

    // C. Comment Button + Comment Count
    curY += btnH + px(18.0f) + gap;
    ImGui::SetCursorScreenPos(ImVec2(actionX, curY));
    if (ImGui::Button("Comment", ImVec2(actionW, btnH))) {
        cur.commentCount++;
    }

    char comStr[16];
    std::snprintf(comStr, sizeof(comStr), "%d", cur.commentCount);
    ImVec2 cSz = ImGui::CalcTextSize(comStr);
    ImGui::SetCursorScreenPos(ImVec2(actionX + (actionW - cSz.x) * 0.5f, curY + btnH + px(1.0f)));
    ImGui::TextDisabled("%s", comStr);

    // D. Share Button
    curY += btnH + px(18.0f) + gap;
    ImGui::SetCursorScreenPos(ImVec2(actionX, curY));
    ImGui::Button("Share", ImVec2(actionW, btnH));

    // E. Down Button (Next)
    curY += btnH + gap;
    ImGui::SetCursorScreenPos(ImVec2(actionX, curY));
    if (ImGui::Button("v", ImVec2(actionW, btnH))) {
        gCurrentReel = (gCurrentReel + 1) % 5;
    }

    ImGui::PopStyleVar();
    ImGui::End();
}