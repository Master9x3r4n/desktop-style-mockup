#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F // missing from Windows' old gl.h
#endif

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <memory>
#include <string>
#include <vector>

// =====================================================================
//  CSOPESY Desktop-Style OS Mock-up  (GLFW + OpenGL3 + Dear ImGui)
//  Layers, drawn back to front every frame:
//    1. Desktop   : wallpaper + clock + PWR button
//    2. Apps      : File Explorer, System Info, Task Manager
//    3. Taskbar   : Start button + app buttons
//    4. Start menu: pops up above the taskbar
// =====================================================================

// ---------------------------------------------------------------------
//  UI scale: every hard-coded pixel size goes through px()
// ---------------------------------------------------------------------
static const float kScale = 1.4f;       // overall GUI size multiplier
static const float kFontPx = 22.0f;     // base font size in pixels
static inline float px(float v) { return v * kScale; }
static inline float TaskbarH() { return px(56.0f); }

// ---------------------------------------------------------------------
//  State
// ---------------------------------------------------------------------
struct Desktop {
    bool showFiles = false;
    bool showSysInfo = false;
    bool showTaskMgr = false;
    bool startOpen = false;
    bool startJustOpened = false;
    bool shutdownRequested = false;
};

static bool gStartButtonHovered = false; // set by taskbar, read by start menu

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

static float DummyCpu(int i, double t) {
    float v = kProcesses[i].baseCpu * (1.0f + 0.25f * (float)std::sin(t * 1.3 + i * 1.7));
    return v < 0.0f ? 0.0f : v;
}
static int DummyMem(int i, double t) {
    return kProcesses[i].baseMemMB + (int)(6.0 * std::sin(t * 0.7 + i));
}

static std::string Lower(std::string s) {
    for (auto& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

// ---------------------------------------------------------------------
//  Fonts
// ---------------------------------------------------------------------
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
            "wallpaper/wallpaper.png",
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

    // ---- Fallback: procedural wallpaper ----
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

    ImGui::SetNextWindowPos(ImVec2(size.x - px(12.0f), px(12.0f)), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.45f);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, px(10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(14), px(10)));
    ImGui::Begin("##clock_power", nullptr, flags);

    std::time_t now = std::time(nullptr);
    std::tm* lt = std::localtime(&now);
    char timeBuf[32], dateBuf[48];
    std::strftime(timeBuf, sizeof(timeBuf), "%I:%M:%S %p", lt);
    std::strftime(dateBuf, sizeof(dateBuf), "%a, %b %d %Y", lt);

    ImGui::BeginGroup();
    ImGui::SetWindowFontScale(1.4f);
    ImGui::TextUnformatted(timeBuf);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::TextDisabled("%s", dateBuf);
    ImGui::EndGroup();

    ImGui::SameLine(0.0f, px(16));

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.10f, 0.10f, 1.0f));
    if (ImGui::Button("PWR", ImVec2(px(60), px(52))))
        d.shutdownRequested = true;
    ImGui::PopStyleColor(3);

    ImGui::End();
    ImGui::PopStyleVar(2);
}

// ---------------------------------------------------------------------
//  Mock file system (for the File Explorer)
// ---------------------------------------------------------------------
struct FsNode {
    std::string name, type, size, modified;
    bool folder = false;
    FsNode* parent = nullptr;
    std::vector<FsNode*> kids;
};

static std::vector<std::unique_ptr<FsNode>> gFsPool;

static FsNode* FsAdd(FsNode* parent, const char* name, bool folder, const char* type,
                     const char* size, const char* modified) {
    gFsPool.emplace_back(new FsNode());
    FsNode* n = gFsPool.back().get();
    n->name = name;
    n->folder = folder;
    n->type = type;
    n->size = size;
    n->modified = modified;
    n->parent = parent;
    if (parent) parent->kids.push_back(n);
    return n;
}
static FsNode* Folder(FsNode* parent, const char* name, const char* modified = "2026-09-28 10:15 AM") {
    return FsAdd(parent, name, true, "File folder", "", modified);
}
static FsNode* File(FsNode* parent, const char* name, const char* type, const char* size,
                    const char* modified = "2026-10-01 02:30 PM") {
    return FsAdd(parent, name, false, type, size, modified);
}

struct Explorer {
    FsNode* root = nullptr;
    FsNode* current = nullptr;
    FsNode* selected = nullptr;
    FsNode *desktop = nullptr, *downloads = nullptr, *documents = nullptr, *pictures = nullptr,
           *music = nullptr, *cDrive = nullptr, *dDrive = nullptr;
    std::vector<FsNode*> history;
    int histIdx = -1;
    char filter[64] = "";
};
static Explorer gEx;

static void BuildFs() {
    Explorer& e = gEx;
    e.root = Folder(nullptr, "This PC", "");
    e.cDrive = Folder(e.root, "Local Disk (C:)", "");
    e.dDrive = Folder(e.root, "Data (D:)", "");
    e.cDrive->type = "Local Disk";
    e.dDrive->type = "Local Disk";

    FsNode* users = Folder(e.cDrive, "Users");
    FsNode* guest = Folder(users, "Guest");
    Folder(e.cDrive, "Program Files");
    Folder(e.cDrive, "Windows");

    e.desktop   = Folder(guest, "Desktop");
    e.documents = Folder(guest, "Documents");
    e.downloads = Folder(guest, "Downloads");
    e.pictures  = Folder(guest, "Pictures");
    e.music     = Folder(guest, "Music");

    File(e.desktop, "CSOPESY Shell.lnk", "Shortcut", "2 KB");

    FsNode* reports = Folder(e.documents, "Reports");
    FsNode* csopesy = Folder(e.documents, "CSOPESY");
    File(reports, "Week1.docx", "Microsoft Word Document", "48 KB");
    File(reports, "Week2.docx", "Microsoft Word Document", "52 KB");
    File(csopesy, "main.cpp", "C++ Source File", "14 KB");
    File(csopesy, "CMakeLists.txt", "Text Document", "2 KB");
    File(e.documents, "notes.txt", "Text Document", "2 KB");
    File(e.documents, "scheduler.cpp", "C++ Source File", "14 KB", "2026-10-03 09:12 AM");
    File(e.documents, "output2.pptx", "PowerPoint Presentation", "3,812 KB", "2026-10-05 08:45 PM");

    File(e.downloads, "installer.exe", "Application", "48,210 KB");
    File(e.downloads, "report.pdf", "PDF Document", "920 KB");
    File(e.downloads, "wallpaper.png", "PNG File", "1,240 KB");

    Folder(e.pictures, "Screenshots");
    Folder(e.pictures, "Vacation");
    File(e.pictures, "wallpaper.png", "PNG File", "1,240 KB");
    File(e.pictures, "photo1.jpg", "JPG File", "2,310 KB");

    File(e.music, "song.mp3", "MP3 File", "4,120 KB");
    File(e.music, "playlist.m3u", "Playlist", "1 KB");

    FsNode* projects = Folder(e.dDrive, "Projects");
    Folder(projects, "OS-Mock-Up");
    Folder(e.dDrive, "Backups");
    File(e.dDrive, "archive.zip", "Compressed Folder", "102,400 KB");

    e.current = e.documents;
    e.history = {e.documents};
    e.histIdx = 0;
}

static void ExNavigate(FsNode* n) {
    if (!n || !n->folder || n == gEx.current) return;
    gEx.history.resize(gEx.histIdx + 1);
    gEx.history.push_back(n);
    gEx.histIdx++;
    gEx.current = n;
    gEx.selected = nullptr;
    gEx.filter[0] = '\0';
}

// Small hand-drawn icons (the default font has no emoji)
static void DrawFolderIcon(ImDrawList* dl, ImVec2 p, float h) {
    float w = h * 1.15f;
    dl->AddRectFilled(ImVec2(p.x, p.y + h * 0.12f), ImVec2(p.x + w * 0.45f, p.y + h * 0.40f),
                      IM_COL32(230, 175, 50, 255), 2.0f);
    dl->AddRectFilled(ImVec2(p.x, p.y + h * 0.26f), ImVec2(p.x + w, p.y + h * 0.90f),
                      IM_COL32(250, 205, 80, 255), 2.0f);
}

static ImU32 AccentFor(const std::string& name) {
    size_t dot = name.find_last_of('.');
    std::string ext = dot == std::string::npos ? "" : Lower(name.substr(dot + 1));
    if (ext == "cpp" || ext == "h" || ext == "c") return IM_COL32(60, 130, 230, 255);
    if (ext == "pptx") return IM_COL32(230, 120, 40, 255);
    if (ext == "docx") return IM_COL32(50, 100, 200, 255);
    if (ext == "pdf") return IM_COL32(210, 55, 55, 255);
    if (ext == "png" || ext == "jpg") return IM_COL32(60, 180, 100, 255);
    if (ext == "exe") return IM_COL32(140, 90, 200, 255);
    if (ext == "mp3" || ext == "m3u") return IM_COL32(220, 90, 160, 255);
    if (ext == "zip") return IM_COL32(200, 170, 60, 255);
    return IM_COL32(130, 130, 140, 255);
}

static void DrawFileIcon(ImDrawList* dl, ImVec2 p, float h, ImU32 accent) {
    float w = h * 0.8f, x = p.x + h * 0.1f, y = p.y + h * 0.05f;
    dl->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h * 0.9f), IM_COL32(238, 238, 244, 255), 2.0f);
    dl->AddRect(ImVec2(x, y), ImVec2(x + w, y + h * 0.9f), IM_COL32(150, 150, 160, 255), 2.0f);
    dl->AddTriangleFilled(ImVec2(x + w * 0.6f, y), ImVec2(x + w, y + w * 0.4f),
                          ImVec2(x + w * 0.6f, y + w * 0.4f), IM_COL32(190, 190, 200, 255));
    dl->AddRectFilled(ImVec2(x + w * 0.18f, y + h * 0.55f), ImVec2(x + w * 0.82f, y + h * 0.72f), accent);
}

static void DrawNodeIcon(ImDrawList* dl, ImVec2 p, float h, const FsNode* n) {
    if (n->folder) DrawFolderIcon(dl, p, h);
    else DrawFileIcon(dl, p, h, AccentFor(n->name));
}

// Navigation-pane entry (folder icon + label)
static void NavItem(const char* label, FsNode* target, FsNode*& goTo) {
    ImGui::PushID(label);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float h = ImGui::GetTextLineHeight();
    if (ImGui::Selectable("##nav", gEx.current == target, 0, ImVec2(0, h + px(6))))
        goTo = target;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    DrawFolderIcon(dl, ImVec2(p.x + px(4), p.y + px(3)), h);
    dl->AddText(ImVec2(p.x + px(4) + h * 1.5f, p.y + px(3)), ImGui::GetColorU32(ImGuiCol_Text), label);
    ImGui::PopID();
}

// ---------------------------------------------------------------------
//  Layer 2: Apps
// ---------------------------------------------------------------------
static void DrawFilesWindow(bool& open) {
    if (!gEx.root) BuildFs();

    ImGui::SetNextWindowSize(ImVec2(px(820), px(520)), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(px(60), px(50)), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("File Explorer", &open)) { ImGui::End(); return; }

    ImGuiStyle& st = ImGui::GetStyle();
    FsNode* goTo = nullptr;

    // ---- Row 1: back / forward / up, address bar, search ----
    bool canBack = gEx.histIdx > 0;
    bool canFwd = gEx.histIdx < (int)gEx.history.size() - 1;
    bool canUp = gEx.current->parent != nullptr;

    ImGui::BeginDisabled(!canBack);
    if (ImGui::Button("<", ImVec2(px(36), 0))) {
        gEx.histIdx--;
        gEx.current = gEx.history[gEx.histIdx];
        gEx.selected = nullptr;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!canFwd);
    if (ImGui::Button(">", ImVec2(px(36), 0))) {
        gEx.histIdx++;
        gEx.current = gEx.history[gEx.histIdx];
        gEx.selected = nullptr;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!canUp);
    if (ImGui::Button("^", ImVec2(px(36), 0))) goTo = gEx.current->parent;
    ImGui::EndDisabled();
    ImGui::SameLine();

    // Address bar with clickable breadcrumbs
    float searchW = px(220);
    float addrW = ImGui::GetContentRegionAvail().x - searchW - st.ItemSpacing.x;
    if (addrW < px(120)) addrW = px(120);
    ImGui::BeginChild("addr", ImVec2(addrW, ImGui::GetFrameHeight() + st.WindowPadding.y * 2.0f),
                      ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        std::vector<FsNode*> chain;
        for (FsNode* n = gEx.current; n; n = n->parent) chain.insert(chain.begin(), n);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        for (size_t i = 0; i < chain.size(); i++) {
            ImGui::PushID((int)i);
            if (i > 0) { ImGui::TextDisabled(">"); ImGui::SameLine(0, px(2)); }
            if (ImGui::SmallButton(chain[i]->name.c_str())) goTo = chain[i];
            if (i + 1 < chain.size()) ImGui::SameLine(0, px(2));
            ImGui::PopID();
        }
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    char hint[96];
    std::snprintf(hint, sizeof(hint), "Search %s", gEx.current->name.c_str());
    ImGui::InputTextWithHint("##search", hint, gEx.filter, sizeof(gEx.filter));

    // ---- Row 2: command bar (placeholders) ----
    const char* cmds[] = {"New folder", "Cut", "Copy", "Paste", "Rename", "Delete", "Sort", "View"};
    for (int i = 0; i < 8; i++) {
        if (i > 0) ImGui::SameLine();
        ImGui::Button(cmds[i]);
    }
    ImGui::Separator();

    // ---- Body: navigation pane + file list ----
    float statusH = ImGui::GetFrameHeightWithSpacing();

    ImGui::BeginChild("nav", ImVec2(px(210), -statusH), ImGuiChildFlags_Borders);
    ImGui::TextDisabled("Quick access");
    NavItem("Desktop", gEx.desktop, goTo);
    NavItem("Downloads", gEx.downloads, goTo);
    NavItem("Documents", gEx.documents, goTo);
    NavItem("Pictures", gEx.pictures, goTo);
    NavItem("Music", gEx.music, goTo);
    ImGui::Spacing();
    ImGui::TextDisabled("This PC");
    NavItem("Local Disk (C:)", gEx.cDrive, goTo);
    NavItem("Data (D:)", gEx.dDrive, goTo);
    ImGui::EndChild();

    ImGui::SameLine();

    int shown = 0;
    ImGui::BeginChild("files", ImVec2(0, -statusH), ImGuiChildFlags_Borders);
    {
        ImGuiTableFlags tf = ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                             ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV;
        if (ImGui::BeginTable("explorer", 4, tf)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Date modified", ImGuiTableColumnFlags_WidthFixed, px(190));
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, px(190));
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, px(90));
            ImGui::TableHeadersRow();

            std::string needle = Lower(gEx.filter);
            float h = ImGui::GetTextLineHeight();
            float rowH = h + px(6);
            ImDrawList* dl = ImGui::GetWindowDrawList();

            for (FsNode* kid : gEx.current->kids) {
                if (!needle.empty() && Lower(kid->name).find(needle) == std::string::npos) continue;
                shown++;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(kid);
                ImVec2 p = ImGui::GetCursorScreenPos();
                if (ImGui::Selectable("##row", gEx.selected == kid,
                                      ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick,
                                      ImVec2(0, rowH)))
                    gEx.selected = kid;
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && kid->folder)
                    goTo = kid;
                DrawNodeIcon(dl, ImVec2(p.x + px(2), p.y + px(3)), h, kid);
                dl->AddText(ImVec2(p.x + px(2) + h * 1.5f, p.y + px(3)),
                            ImGui::GetColorU32(ImGuiCol_Text), kid->name.c_str());
                ImGui::PopID();

                ImGui::TableSetColumnIndex(1);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + px(3));
                ImGui::TextUnformatted(kid->modified.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + px(3));
                ImGui::TextUnformatted(kid->type.c_str());
                ImGui::TableSetColumnIndex(3);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + px(3));
                ImGui::TextUnformatted(kid->size.c_str());
            }
            ImGui::EndTable();
        }
        if (shown == 0) ImGui::TextDisabled(gEx.filter[0] ? "No items match your search." : "This folder is empty.");
    }
    ImGui::EndChild();

    // ---- Status bar ----
    ImGui::Text("%d item%s", shown, shown == 1 ? "" : "s");
    if (gEx.selected) {
        ImGui::SameLine();
        ImGui::TextDisabled("|   1 item selected");
    }

    if (goTo) ExNavigate(goTo);
    ImGui::End();
}

static void DrawSysInfoWindow(bool& open) {
    ImGui::SetNextWindowSize(ImVec2(px(420), px(300)), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(px(160), px(110)), ImGuiCond_FirstUseEver);
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
    ImGui::SetNextWindowSize(ImVec2(px(620), px(460)), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(px(280), px(40)), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Task Manager", &open)) { ImGui::End(); return; }

    double t = glfwGetTime();

    float totalCpu = 0.0f;
    int totalMem = 0;
    for (int i = 0; i < kProcessCount; i++) { totalCpu += DummyCpu(i, t); totalMem += DummyMem(i, t); }
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

// ---------------------------------------------------------------------
//  Layer 3: Taskbar
// ---------------------------------------------------------------------
static bool TaskbarButton(const char* label, bool isOpen) {
    if (isOpen) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.30f, 0.45f, 0.75f, 1.0f));
    bool clicked = ImGui::Button(label, ImVec2(px(130), px(40)));
    if (isOpen) ImGui::PopStyleColor();

    if (isOpen) {
        ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(a.x + px(14), b.y + px(3)),
                                                  ImVec2(b.x - px(14), b.y + px(5)),
                                                  IM_COL32(120, 190, 255, 255), 2.0f);
    }
    return clicked;
}

// The "Start" button: 4-square logo + CSOPESY label
static bool StartButton(bool active) {
    ImGui::PushStyleColor(ImGuiCol_Button, active ? ImVec4(0.20f, 0.45f, 0.90f, 1.0f)
                                                  : ImVec4(0.16f, 0.36f, 0.72f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.52f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.14f, 0.32f, 0.65f, 1.0f));
    bool clicked = ImGui::Button("##start", ImVec2(px(150), px(40)));
    gStartButtonHovered = ImGui::IsItemHovered();
    ImGui::PopStyleColor(3);

    ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float sq = px(8), gap = px(2);
    float lx = a.x + px(14);
    float ly = (a.y + b.y) * 0.5f - sq - gap * 0.5f;
    dl->AddRectFilled(ImVec2(lx, ly), ImVec2(lx + sq, ly + sq), IM_COL32(120, 210, 255, 255), 1.0f);
    dl->AddRectFilled(ImVec2(lx + sq + gap, ly), ImVec2(lx + 2 * sq + gap, ly + sq), IM_COL32(150, 230, 255, 255), 1.0f);
    dl->AddRectFilled(ImVec2(lx, ly + sq + gap), ImVec2(lx + sq, ly + 2 * sq + gap), IM_COL32(90, 190, 245, 255), 1.0f);
    dl->AddRectFilled(ImVec2(lx + sq + gap, ly + sq + gap), ImVec2(lx + 2 * sq + gap, ly + 2 * sq + gap), IM_COL32(120, 210, 255, 255), 1.0f);

    float tx = lx + 2 * sq + gap + px(12);
    float ty = a.y + ((b.y - a.y) - ImGui::GetFontSize()) * 0.5f;
    dl->AddText(ImVec2(tx, ty), IM_COL32(255, 255, 255, 255), "CSOPESY");
    return clicked;
}

static void DrawTaskbar(Desktop& d) {
    ImVec2 size = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0, size.y - TaskbarH()));
    ImGui::SetNextWindowSize(ImVec2(size.x, TaskbarH()));
    ImGui::SetNextWindowBgAlpha(0.88f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(10), px(8)));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, px(8));
    ImGui::Begin("##taskbar", nullptr, flags);

    if (StartButton(d.startOpen)) {
        d.startOpen = !d.startOpen;
        d.startJustOpened = d.startOpen;
    }
    ImGui::SameLine(0.0f, px(18));

    if (TaskbarButton("Files", d.showFiles))          d.showFiles = !d.showFiles;
    ImGui::SameLine();
    if (TaskbarButton("System Info", d.showSysInfo))  d.showSysInfo = !d.showSysInfo;
    ImGui::SameLine();
    if (TaskbarButton("Task Manager", d.showTaskMgr)) d.showTaskMgr = !d.showTaskMgr;

    ImGui::End();
    ImGui::PopStyleVar(3);
}

// ---------------------------------------------------------------------
//  Layer 4: Start menu
// ---------------------------------------------------------------------
// One pinned-app row: coloured tile + title + subtitle. Returns true on click.
static bool StartMenuApp(const char* title, const char* subtitle, ImU32 color) {
    ImGui::PushID(title);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float rowH = px(54);
    bool clicked = ImGui::Selectable("##app", false, 0, ImVec2(0, rowH));
    ImDrawList* dl = ImGui::GetWindowDrawList();

    float tile = px(38);
    ImVec2 t0(p.x + px(6), p.y + (rowH - tile) * 0.5f);
    dl->AddRectFilled(t0, ImVec2(t0.x + tile, t0.y + tile), color, px(8));
    char letter[2] = {title[0], 0};
    ImVec2 ls = ImGui::CalcTextSize(letter);
    dl->AddText(ImVec2(t0.x + (tile - ls.x) * 0.5f, t0.y + (tile - ls.y) * 0.5f),
                IM_COL32(255, 255, 255, 255), letter);

    float tx = t0.x + tile + px(12);
    dl->AddText(ImVec2(tx, p.y + px(6)), ImGui::GetColorU32(ImGuiCol_Text), title);
    dl->AddText(ImVec2(tx, p.y + px(6) + ImGui::GetFontSize() + px(1)),
                ImGui::GetColorU32(ImGuiCol_TextDisabled), subtitle);
    ImGui::PopID();
    return clicked;
}

static void DrawStartMenu(Desktop& d) {
    if (!d.startOpen) return;

    ImVec2 size = ImGui::GetIO().DisplaySize;
    float w = px(360), h = px(480);
    ImGui::SetNextWindowPos(ImVec2(px(8), size.y - TaskbarH() - h - px(8)));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::SetNextWindowBgAlpha(0.96f);
    if (d.startJustOpened) {
        ImGui::SetNextWindowFocus();
        d.startJustOpened = false;
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, px(10));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(14), px(12)));
    ImGui::Begin("##startmenu", nullptr, flags);

    ImGui::SetWindowFontScale(1.2f);
    ImGui::TextColored(ImVec4(0.55f, 0.75f, 1.0f, 1.0f), "CSOPESY OS");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    static char searchBuf[64] = "";
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##startsearch", "Type here to search", searchBuf, sizeof(searchBuf));

    ImGui::Spacing();
    ImGui::TextDisabled("Pinned");
    if (StartMenuApp("Files", "Browse folders and documents", IM_COL32(230, 160, 40, 255))) {
        d.showFiles = true; d.startOpen = false; ImGui::SetWindowFocus("File Explorer");
    }
    if (StartMenuApp("System Info", "Version and hardware details", IM_COL32(60, 140, 220, 255))) {
        d.showSysInfo = true; d.startOpen = false; ImGui::SetWindowFocus("System Info");
    }
    if (StartMenuApp("Task Manager", "Processes and performance", IM_COL32(60, 170, 110, 255))) {
        d.showTaskMgr = true; d.startOpen = false; ImGui::SetWindowFocus("Task Manager");
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Recommended");
    ImGui::BulletText("scheduler.cpp");
    ImGui::BulletText("output2.pptx");

    // Footer: user name on the left, power on the right
    ImGui::SetCursorPosY(h - px(64));
    ImGui::Separator();
    ImGui::Spacing();
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        float r = px(15);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddCircleFilled(ImVec2(p.x + r, p.y + r), r, IM_COL32(90, 130, 200, 255), 32);
        dl->AddCircleFilled(ImVec2(p.x + r, p.y + r * 0.8f), r * 0.35f, IM_COL32(255, 255, 255, 230), 16);
        dl->AddCircleFilled(ImVec2(p.x + r, p.y + r * 1.75f), r * 0.65f, IM_COL32(255, 255, 255, 230), 16);
        dl->PushClipRect(p, ImVec2(p.x + r * 2, p.y + r * 2), true);
        dl->PopClipRect();
        ImGui::SetCursorScreenPos(ImVec2(p.x + r * 2 + px(10), p.y + r - ImGui::GetFontSize() * 0.5f));
        ImGui::TextUnformatted("Guest");
        ImGui::SameLine(ImGui::GetWindowWidth() - px(76) - ImGui::GetStyle().WindowPadding.x);
        ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, p.y - px(2)));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.25f, 0.25f, 1.0f));
        if (ImGui::Button("PWR", ImVec2(px(60), px(36)))) d.shutdownRequested = true;
        ImGui::PopStyleColor(2);
    }

    // Close when clicking elsewhere or pressing Esc
    bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows |
                                          ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    if ((ImGui::IsMouseClicked(0) && !hovered && !gStartButtonHovered) ||
        ImGui::IsKeyPressed(ImGuiKey_Escape))
        d.startOpen = false;

    ImGui::End();
    ImGui::PopStyleVar(2);
}

// ---------------------------------------------------------------------
//  Main
// ---------------------------------------------------------------------
int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE); // fill the screen like a desktop

    GLFWwindow* window = glfwCreateWindow(1600, 900, "CSOPESY OS", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // don't persist window layout between runs

    LoadFonts(io, kFontPx);

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(kScale);
    style.WindowRounding = px(8.0f);
    style.FrameRounding = px(4.0f);

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

        // 3. Taskbar, 4. Start menu
        DrawTaskbar(desktop);
        DrawStartMenu(desktop);

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