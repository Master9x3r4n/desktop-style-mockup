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

static CalcState gCalculator;
static void ClearCalculator() {
    gCalculator.display = "0";
    gCalculator.op1 = 0.0;
    gCalculator.pendingOp = 0;
    gCalculator.waitingForOp2 = false;
    gCalculator.hasDecimal = false;
}

static void InputDigit(char digit) {
    if (gCalculator.waitingForOp2 || gCalculator.display == "0" || gCalculator.display == "Error") {
        gCalculator.display = std::string(1, digit);
        gCalculator.waitingForOp2 = false;
        gCalculator.hasDecimal = false;
    } else {
        if (gCalculator.display.length() < 12) {
            gCalculator.display += digit;
        }
    }
}

static void InputDecimal() {
    if (gCalculator.waitingForOp2 || gCalculator.display == "Error") {
        gCalculator.display = "0.";
        gCalculator.waitingForOp2 = false;
        gCalculator.hasDecimal = true;
    } else if (!gCalculator.hasDecimal) {
        gCalculator.display += ".";
        gCalculator.hasDecimal = true;
    }
}

static void Calculate() {
    if (gCalculator.pendingOp == 0) return;
    double op2 = std::stod(gCalculator.display);
    double result = 0.0;

    if (gCalculator.pendingOp == '+') result = gCalculator.op1 + op2;
    else if (gCalculator.pendingOp == '-') result = gCalculator.op1 - op2;
    else if (gCalculator.pendingOp == '*') result = gCalculator.op1 * op2;
    else if (gCalculator.pendingOp == '/') {
        if (std::abs(op2) < 0.0000001) {
            gCalculator.display = "Error";
            gCalculator.pendingOp = 0;
            return;
        }
        result = gCalculator.op1 / op2;
    }

    std::ostringstream ss;
    ss.precision(8);
    ss << result;
    gCalculator.display = ss.str();
    gCalculator.op1 = result;
    gCalculator.pendingOp = 0;
    gCalculator.waitingForOp2 = true;
}

static void SetOperation(char op) {
    if (gCalculator.pendingOp != 0 && !gCalculator.waitingForOp2) {
        Calculate();
    }
    gCalculator.op1 = std::stod(gCalculator.display);
    gCalculator.pendingOp = op;
    gCalculator.waitingForOp2 = true;
    gCalculator.hasDecimal = false;
}

void DrawCalculatorWindow(bool& open) {
    ImGui::SetNextWindowSize(ImVec2(px(320), px(440)), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Calculator", &open, ImGuiWindowFlags_NoResize)) {
        ImGui::End();
        return;
    }

    ImVec2 dispSz(ImGui::GetContentRegionAvail().x, px(54.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.09f, 0.12f, 0.85f));
    ImGui::BeginChild("calc_disp", dispSz, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::SetWindowFontScale(1.35f);
        ImVec2 tSz = ImGui::CalcTextSize(gCalculator.display.c_str());
        ImGui::SetCursorPos(ImVec2(dispSz.x - tSz.x - px(14), (dispSz.y - tSz.y) * 0.5f));
        ImGui::TextUnformatted(gCalculator.display.c_str());
        ImGui::SetWindowFontScale(1.0f);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::Spacing();

    float sp = px(8.0f);
    float availW = ImGui::GetContentRegionAvail().x;
    float btnW = (availW - 3.0f * sp) / 4.0f;
    float btnH = px(48.0f);
    float dblW = btnW * 2.0f + sp;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(sp, sp));
    auto NumBtn = [&](const char* label, char d) {
        if (ImGui::Button(label, ImVec2(btnW, btnH))) InputDigit(d);
    };

    auto OpBtn = [&](const char* label, char op) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.35f, 0.55f, 0.70f));
        if (ImGui::Button(label, ImVec2(btnW, btnH))) SetOperation(op);
        ImGui::PopStyleColor();
    };

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 0.75f));
    if (ImGui::Button("C", ImVec2(dblW, btnH))) ClearCalculator();
    ImGui::PopStyleColor();
    ImGui::SameLine();
    if (ImGui::Button("+/-", ImVec2(btnW, btnH))) {
        if (gCalculator.display != "0" && gCalculator.display != "Error") {
            if (gCalculator.display[0] == '-') gCalculator.display.erase(0, 1);
            else gCalculator.display.insert(0, "-");
        }
    }
    ImGui::SameLine();
    OpBtn("/", '/');
    NumBtn("7", '7'); ImGui::SameLine();
    NumBtn("8", '8'); ImGui::SameLine();
    NumBtn("9", '9'); ImGui::SameLine();
    OpBtn("*", '*');
    NumBtn("4", '4'); ImGui::SameLine();
    NumBtn("5", '5'); ImGui::SameLine();
    NumBtn("6", '6'); ImGui::SameLine();
    OpBtn("-", '-');
    NumBtn("1", '1'); ImGui::SameLine();
    NumBtn("2", '2'); ImGui::SameLine();
    NumBtn("3", '3'); ImGui::SameLine();
    OpBtn("+", '+');
    if (ImGui::Button("0", ImVec2(dblW, btnH))) InputDigit('0');
    ImGui::SameLine();
    if (ImGui::Button(".", ImVec2(btnW, btnH))) InputDecimal();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, GetAccentColor(0));
    if (ImGui::Button("=", ImVec2(btnW, btnH))) Calculate();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::End();
}