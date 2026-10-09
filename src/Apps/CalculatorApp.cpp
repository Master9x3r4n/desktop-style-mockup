#include "CalculatorApp.h"
#include <string>
#include <sstream>
#include <cmath>

struct CalcState {
    std::string display = "0";
    double op1 = 0.0;
    char pendingOp = 0;
    bool waitingForOp2 = false;
    bool hasDecimal = false;
};

static CalcState gCalc;

static void CalcClear() {
    gCalc.display = "0";
    gCalc.op1 = 0.0;
    gCalc.pendingOp = 0;
    gCalc.waitingForOp2 = false;
    gCalc.hasDecimal = false;
}

static void CalcInputDigit(char digit) {
    if (gCalc.waitingForOp2 || gCalc.display == "0" || gCalc.display == "Error") {
        gCalc.display = std::string(1, digit);
        gCalc.waitingForOp2 = false;
        gCalc.hasDecimal = false;
    } else {
        if (gCalc.display.length() < 12) {
            gCalc.display += digit;
        }
    }
}

static void CalcInputDecimal() {
    if (gCalc.waitingForOp2 || gCalc.display == "Error") {
        gCalc.display = "0.";
        gCalc.waitingForOp2 = false;
        gCalc.hasDecimal = true;
    } else if (!gCalc.hasDecimal) {
        gCalc.display += ".";
        gCalc.hasDecimal = true;
    }
}

static void CalcExecute() {
    if (gCalc.pendingOp == 0) return;
    double op2 = std::stod(gCalc.display);
    double result = 0.0;

    if (gCalc.pendingOp == '+') result = gCalc.op1 + op2;
    else if (gCalc.pendingOp == '-') result = gCalc.op1 - op2;
    else if (gCalc.pendingOp == '*') result = gCalc.op1 * op2;
    else if (gCalc.pendingOp == '/') {
        if (std::abs(op2) < 0.0000001) {
            gCalc.display = "Error";
            gCalc.pendingOp = 0;
            return;
        }
        result = gCalc.op1 / op2;
    }

    std::ostringstream ss;
    ss.precision(8);
    ss << result;
    gCalc.display = ss.str();
    gCalc.op1 = result;
    gCalc.pendingOp = 0;
    gCalc.waitingForOp2 = true;
}

static void CalcSetOp(char op) {
    if (gCalc.pendingOp != 0 && !gCalc.waitingForOp2) {
        CalcExecute();
    }
    gCalc.op1 = std::stod(gCalc.display);
    gCalc.pendingOp = op;
    gCalc.waitingForOp2 = true;
    gCalc.hasDecimal = false;
}

void DrawCalculatorWindow(bool& open) {
    ImGui::SetNextWindowSize(ImVec2(px(320), px(440)), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Calculator", &open, ImGuiWindowFlags_NoResize)) {
        ImGui::End();
        return;
    }

    // 1. Digital Display Box
    ImVec2 dispSz(ImGui::GetContentRegionAvail().x, px(54.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.09f, 0.12f, 0.85f));
    ImGui::BeginChild("calc_disp", dispSz, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::SetWindowFontScale(1.35f);
        ImVec2 tSz = ImGui::CalcTextSize(gCalc.display.c_str());
        ImGui::SetCursorPos(ImVec2(dispSz.x - tSz.x - px(14), (dispSz.y - tSz.y) * 0.5f));
        ImGui::TextUnformatted(gCalc.display.c_str());
        ImGui::SetWindowFontScale(1.0f);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // 2. Consistent 4-column keypad
    float sp = px(8.0f);
    float availW = ImGui::GetContentRegionAvail().x;
    float btnW = (availW - 3.0f * sp) / 4.0f;
    float btnH = px(48.0f);
    float dblW = btnW * 2.0f + sp;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(sp, sp));

    auto NumBtn = [&](const char* label, char d) {
        if (ImGui::Button(label, ImVec2(btnW, btnH))) CalcInputDigit(d);
    };

    auto OpBtn = [&](const char* label, char op) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.35f, 0.55f, 0.70f));
        if (ImGui::Button(label, ImVec2(btnW, btnH))) CalcSetOp(op);
        ImGui::PopStyleColor();
    };

    // Row 1: C (double wide) | +/- | /
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 0.75f));
    if (ImGui::Button("C", ImVec2(dblW, btnH))) CalcClear();
    ImGui::PopStyleColor();
    ImGui::SameLine();
    if (ImGui::Button("+/-", ImVec2(btnW, btnH))) {
        if (gCalc.display != "0" && gCalc.display != "Error") {
            if (gCalc.display[0] == '-') gCalc.display.erase(0, 1);
            else gCalc.display.insert(0, "-");
        }
    }
    ImGui::SameLine();
    OpBtn("/", '/');

    // Row 2: 7 | 8 | 9 | *
    NumBtn("7", '7'); ImGui::SameLine();
    NumBtn("8", '8'); ImGui::SameLine();
    NumBtn("9", '9'); ImGui::SameLine();
    OpBtn("*", '*');

    // Row 3: 4 | 5 | 6 | -
    NumBtn("4", '4'); ImGui::SameLine();
    NumBtn("5", '5'); ImGui::SameLine();
    NumBtn("6", '6'); ImGui::SameLine();
    OpBtn("-", '-');

    // Row 4: 1 | 2 | 3 | +
    NumBtn("1", '1'); ImGui::SameLine();
    NumBtn("2", '2'); ImGui::SameLine();
    NumBtn("3", '3'); ImGui::SameLine();
    OpBtn("+", '+');

    // Row 5: 0 (double wide) | . | =
    if (ImGui::Button("0", ImVec2(dblW, btnH))) CalcInputDigit('0');
    ImGui::SameLine();
    if (ImGui::Button(".", ImVec2(btnW, btnH))) CalcInputDecimal();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, GetAccentColor(0));
    if (ImGui::Button("=", ImVec2(btnW, btnH))) CalcExecute();
    ImGui::PopStyleColor();

    ImGui::PopStyleVar();
    ImGui::End();
}