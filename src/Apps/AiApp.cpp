#include "AiApp.h"
#include <string>
#include <vector>
#include <cstdlib>

struct ChatMessage {
    bool isUser;
    std::string text;
};

struct AiState {
    std::vector<ChatMessage> history;
    int remainingTokens = 5;
    char inputBuf[256] = "";
    bool scrollToBottom = false;
};

static AiState gAi;

static const char* kChicaReplies[] = {
    "I hear you! That's a really interesting point, tell me more about it.",
    "Honestly, that sounds like a great plan. Let's see how that works out!",
    "I completely agree with that perspective. What do you think should be our next step?",
    "Haha, that's wild! You always bring up the most fascinating topics.",
    "That makes total sense to me! I'm right here with you on this one.",
    "I'm not 100% sure, but if I had to guess, I'd say you're definitely onto something.",
    "Fascinating thought! Thanks for sharing that with me.",
    "Oh, definitely! We should dive deeper into that whenever you're ready."
};
static const int kChicaReplyCount = sizeof(kChicaReplies) / sizeof(kChicaReplies[0]);

static void SendAiMessage() {
    if (gAi.remainingTokens <= 0 || gAi.inputBuf[0] == '\0') return;

    gAi.history.push_back({true, gAi.inputBuf});
    gAi.remainingTokens--;
    gAi.inputBuf[0] = '\0';

    int pick = std::rand() % kChicaReplyCount;
    gAi.history.push_back({false, kChicaReplies[pick]});
    gAi.scrollToBottom = true;
}

void DrawAiWindow(bool& open) {
    ImGui::SetNextWindowSize(ImVec2(px(460), px(520)), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("ChicaGPT", &open)) {
        ImGui::End();
        return;
    }

    // Header token indicator
    ImGui::TextColored(GetAccentColor(0), "ChicaGPT 4.0");
    ImGui::SameLine(ImGui::GetWindowWidth() - px(180));
    ImGui::TextDisabled("Tokens left: %d / 5", gAi.remainingTokens);
    ImGui::Separator();

    // Chat Message Log
    float inputH = px(44.0f);
    ImGui::BeginChild("chat_log", ImVec2(0, -inputH - px(10)), ImGuiChildFlags_Borders);
    {
        if (gAi.history.empty()) {
            ImGui::Spacing();
            ImGui::TextDisabled("Say hi to ChicaGPT! Start a conversation below.");
        }

        for (const auto& msg : gAi.history) {
            ImGui::Spacing();
            if (msg.isUser) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.8f, 1.0f, 1.0f));
                ImGui::TextUnformatted("You:");
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.55f, 0.75f, 1.0f));
                ImGui::TextUnformatted("ChicaGPT:");
                ImGui::PopStyleColor();
            }
            ImGui::TextWrapped("%s", msg.text.c_str());
        }

        if (gAi.scrollToBottom) {
            ImGui::SetScrollHereY(1.0f);
            gAi.scrollToBottom = false;
        }
    }
    ImGui::EndChild();

    // Input row
    bool tokensExhausted = (gAi.remainingTokens <= 0);
    ImGui::BeginDisabled(tokensExhausted);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - px(130));
    bool enter = ImGui::InputTextWithHint("##ai_input", tokensExhausted ? "Token limit reached." : "Message ChicaGPT...",
                                          gAi.inputBuf, sizeof(gAi.inputBuf), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    if (ImGui::Button("Send", ImVec2(px(60), 0)) || (enter && !tokensExhausted)) {
        SendAiMessage();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(px(60), 0))) {
        gAi.history.clear();
        gAi.remainingTokens = 5;
    }

    ImGui::End();
}