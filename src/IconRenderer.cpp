#include "stb_image.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#include "IconRenderer.h"
#include <unordered_map>
#include <string>

extern float px(float v);

static GLuint LoadIconTexture(const char* filename, int& outW, int& outH) {
    // candidate pathfiles
    std::string p1 = std::string(PROJECT_DIR) + "/assets/icons/" + filename;
    std::string p2 = std::string("assets/icons/") + filename;
    std::string p3 = std::string("../assets/icons/") + filename;

    const char* paths[] = { p1.c_str(), p2.c_str(), p3.c_str() };

    unsigned char* data = nullptr;
    int channels = 0;

    // check all pathfiles for the image
    for (const char* path : paths) {
        data = stbi_load(path, &outW, &outH, &channels, 4);
        if (data) break;
    }

    // use fallback if all pathfiles were not found
    if (!data) 
        return 0;

    // upload icon to gpu
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

void DrawIcon(ImDrawList* dl, ImVec2 iMin, ImVec2 iMax, float iconSz, const char* label, ImU32 color, const char* badge) {
    static std::unordered_map<std::string, GLuint> iconCache;
    std::string key = label ? label : "";

    if (iconCache.find(key) == iconCache.end()) {
        std::string filename = key + ".png";
        int w = 0, h = 0;

        iconCache[key] = LoadIconTexture(filename.c_str(), w, h);
    }

    GLuint tex = iconCache[key];

    // draw holder for icon
    dl->AddRectFilled(ImVec2(iMin.x, iMin.y + px(2)), ImVec2(iMax.x, iMax.y + px(2)), IM_COL32(0, 0, 0, 70), px(10.0f));
    
    // if found icon, draw that
    if (tex != 0) {
        dl->AddImage((ImTextureID)(uintptr_t)tex, iMin, iMax);
        dl->AddRect(iMin, iMax, IM_COL32(255, 255, 255, 40), px(10.f));
    }
    // else, draw the fallback instead
    else {
        dl->AddRectFilled(iMin, iMax, color, px(10.0f));

        dl->AddRect(iMin, iMax, IM_COL32(255, 255, 255, 40), px(10.0f));

        ImVec2 bSz = ImGui::CalcTextSize(badge);
        dl->AddText(ImVec2(iMin.x + (iconSz - bSz.x) * 0.5f, iMin.y + (iconSz - bSz.y) * 0.5f),
                    IM_COL32(255, 255, 255, 255), badge);
    }
}