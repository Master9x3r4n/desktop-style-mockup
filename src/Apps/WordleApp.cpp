#include "WordleApp.h"
#include <string>
#include <vector>
#include <cctype>

struct WordleRow {
    std::string guess;
    int status[5];
};

struct WordleState {
    const std::string solution = "OPESY";
    std::vector<WordleRow> rows;
    char inputBuf[16] = "";
    bool won = false;
    bool gameOver = false;
    std::string statusMsg = "";
};

static WordleState gWordle;

static void ResetWordle() {
    gWordle.rows.clear();
    gWordle.inputBuf[0] = '\0';
    gWordle.won = false;
    gWordle.gameOver = false;
    gWordle.statusMsg = "";
}

static void SubmitGuess() {
    if (gWordle.gameOver) return;

    std::string guess = gWordle.inputBuf;
    for (char& c : guess) c = (char)std::toupper((unsigned char)c);
    if (guess.length() != 5) {
        gWordle.statusMsg = "Guess must be exactly 5 letters!";
        return;
    }

    WordleRow row;
    row.guess = guess;
    std::string sol = gWordle.solution;
    bool solMatched[5] = {false, false, false, false, false};
    for (int i = 0; i < 5; i++) row.status[i] = 1;

    for (int i = 0; i < 5; i++) {
        if (guess[i] == sol[i]) {
            row.status[i] = 3;
            solMatched[i] = true;
        }
    }

    for (int i = 0; i < 5; i++) {
        if (row.status[i] == 3) continue;
        for (int j = 0; j < 5; j++) {
            if (!solMatched[j] && guess[i] == sol[j]) {
                row.status[i] = 2;
                solMatched[j] = true;
                break;
            }
        }
    }

    gWordle.rows.push_back(row);
    gWordle.inputBuf[0] = '\0';
    gWordle.statusMsg = "";

    if (guess == sol) {
        gWordle.won = true;
        gWordle.gameOver = true;
        gWordle.statusMsg = "Impressive! You guessed OPESY!";
    } else if (gWordle.rows.size() >= 6) {
        gWordle.gameOver = true;
        gWordle.statusMsg = "Game Over! The word was OPESY.";
    }
}

void DrawWordleWindow(bool& open) {
    ImGui::SetNextWindowSize(ImVec2(px(380), px(540)), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Wuhdle", &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize)) {
        ImGui::End();
        return;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float boxSz = px(44.0f);
    float gap = px(6.0f);
    float totalGridW = 5 * boxSz + 4 * gap;
    float startX = ImGui::GetCursorScreenPos().x + (ImGui::GetContentRegionAvail().x - totalGridW) * 0.5f;

    for (int r = 0; r < 6; r++) {
        ImVec2 p(startX, ImGui::GetCursorScreenPos().y + r * (boxSz + gap));
        bool hasRow = r < (int)gWordle.rows.size();

        for (int c = 0; c < 5; c++) {
            ImVec2 bMin(p.x + c * (boxSz + gap), p.y);
            ImVec2 bMax(bMin.x + boxSz, bMin.y + boxSz);

            ImU32 fillCol = IM_COL32(30, 32, 42, 255);
            ImU32 borderCol = IM_COL32(65, 70, 85, 255);
            char letter[2] = {0, 0};

            if (hasRow) {
                int status = gWordle.rows[r].status[c];
                letter[0] = gWordle.rows[r].guess[c];

                if (status == 3)      fillCol = IM_COL32(83, 141, 78, 255);
                else if (status == 2) fillCol = IM_COL32(181, 159, 59, 255);
                else                  fillCol = IM_COL32(58, 58, 60, 255);
                borderCol = fillCol;
            }

            dl->AddRectFilled(bMin, bMax, fillCol, px(6.0f));
            dl->AddRect(bMin, bMax, borderCol, px(6.0f), 0, 1.5f);

            if (letter[0]) {
                ImVec2 tSz = ImGui::CalcTextSize(letter);
                dl->AddText(ImVec2(bMin.x + (boxSz - tSz.x) * 0.5f, bMin.y + (boxSz - tSz.y) * 0.5f),
                            IM_COL32(255, 255, 255, 255), letter);
            }
        }
    }

    ImGui::Dummy(ImVec2(0, 6 * (boxSz + gap) + px(10)));

    if (!gWordle.statusMsg.empty()) {
        ImVec4 col = gWordle.won ? ImVec4(0.3f, 0.9f, 0.4f, 1.0f) : ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        ImGui::TextColored(col, "%s", gWordle.statusMsg.c_str());
    } else {
        ImGui::TextDisabled("Guess the 5-letter word (%d/6)", (int)gWordle.rows.size());
    }

    ImGui::Spacing();

    ImGui::BeginDisabled(gWordle.gameOver);
    ImGui::SetNextItemWidth(px(140));
    bool enterPressed = ImGui::InputTextWithHint("##wordinput", "5 LETTERS", gWordle.inputBuf, sizeof(gWordle.inputBuf),
                                                 ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    if ((ImGui::Button("Submit", ImVec2(px(80), 0)) || enterPressed) && !gWordle.gameOver) {
        SubmitGuess();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(px(70), 0))) {
        ResetWordle();
    }

    ImGui::End();
}