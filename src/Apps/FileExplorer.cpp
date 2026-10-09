#include "FileExplorer.h"
#include "Common.h"
#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

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
    File(reports, "STADVDB_NOTES.docx", "Microsoft Word Document", "48 KB");
    File(reports, "CSARCH2_REVIEW_FINAL_FINAL.docx", "Microsoft Word Document", "52 KB");
    File(csopesy, "main.cpp", "C++ Source File", "14 KB");
    File(csopesy, "CMakeLists.txt", "Text Document", "2 KB");
    File(e.documents, "notes.txt", "Text Document", "2 KB");
    File(e.documents, "scheduler.cpp", "C++ Source File", "14 KB", "2026-10-03 09:12 AM");
    File(e.documents, "output2.pptx", "PowerPoint Presentation", "3,812 KB", "2026-10-05 08:45 PM");

    File(e.downloads, "installer.exe", "Application", "48,210 KB");
    File(e.downloads, "report.pdf", "PDF Document", "920 KB");
    File(e.downloads, "wallpaper.png", "PNG File", "1,240 KB");

    Folder(e.pictures, "Screenshots");
    Folder(e.pictures, "Wendyslop Pics");
    File(e.pictures, "wallpaper.png", "PNG File", "1,240 KB");
    File(e.pictures, "photo1.jpg", "JPG File", "2,310 KB");

    File(e.music, "feel.mp3", "MP3 File", "4,120 KB");
    File(e.music, "awesome_mix_vol1.m3u", "Playlist", "1 KB");

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

static std::string Lower(std::string s) {
    for (auto& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

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

void DrawFilesWindow(bool& open) {
    if (!gEx.root) BuildFs();

    ImGui::SetNextWindowSize(ImVec2(px(820), px(520)), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(px(60), px(50)), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("File Explorer", &open)) { ImGui::End(); return; }

    ImGuiStyle& st = ImGui::GetStyle();
    FsNode* goTo = nullptr;

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

    const char* cmds[] = {"New folder", "Cut", "Copy", "Paste", "Rename", "Delete", "Sort", "View"};
    for (int i = 0; i < 8; i++) {
        if (i > 0) ImGui::SameLine();
        ImGui::Button(cmds[i]);
    }
    ImGui::Separator();

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

    ImGui::Text("%d item%s", shown, shown == 1 ? "" : "s");
    if (gEx.selected) {
        ImGui::SameLine();
        ImGui::TextDisabled("|   1 item selected");
    }

    if (goTo) ExNavigate(goTo);
    ImGui::End();
}