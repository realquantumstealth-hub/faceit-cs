#include "renderer.hpp"
#include <algorithm>
#include <cmath>

template<typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

Renderer g_Renderer;

void Renderer::RenderESP() {
    auto players = g_EntitySystem.GetPlayers();
    if (players.empty()) return;

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    static const ImU32 enemyColor = IM_COL32(255, 85, 85, 255);
    static const ImU32 teamColor = IM_COL32(85, 170, 255, 255);
    static const ImU32 skeletonColor = IM_COL32(255, 255, 255, 200);

    for (auto& player : players) {
        if (!player.isValid) continue;

        player.screenPos = g_EntitySystem.WorldToScreen(player.position);
        player.headScreenPos = g_EntitySystem.WorldToScreen(player.headPos);

        if (!player.screenPos.valid && !player.headScreenPos.valid) continue;

        for (auto& bonePair : player.bones.positions) {
            player.bones.screenPositions[bonePair.first] = g_EntitySystem.WorldToScreen(bonePair.second);
        }

        float boxHeight = std::abs(player.screenPos.y - player.headScreenPos.y);
        float boxWidth = boxHeight * 0.4f;
        float boxLeft = player.headScreenPos.x - (boxWidth * 0.5f);
        float boxRight = player.headScreenPos.x + (boxWidth * 0.5f);
        float boxTop = player.headScreenPos.y;
        float boxBottom = player.screenPos.y;

        ImU32 playerColor = (player.team == 2) ? enemyColor : teamColor;

        if (player.screenPos.valid && player.headScreenPos.valid) {
            DrawModernBox(boxLeft, boxTop, boxRight, boxBottom, playerColor);

            DrawModernHealthBar(boxLeft, boxTop, boxBottom, boxWidth, player.health, player.armor);

            DrawModernPlayerInfo(player, boxLeft, boxRight, boxTop, boxBottom);

            DrawModernSkeleton(player.bones, playerColor);
        }
    }
}

void Renderer::DrawModernBox(float left, float top, float right, float bottom, ImU32 color) {
    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    const float cornerRadius = 2.0f;
    const float thickness = 1.5f;

    ImU32 glowColor = (color & 0x00FFFFFF) | 0x30000000;
    drawList->AddRect(
        ImVec2(left - 1, top - 1),
        ImVec2(right + 1, bottom + 1),
        glowColor, cornerRadius + 1.0f, 0, thickness + 1.0f
    );

    drawList->AddRect(
        ImVec2(left - 0.5f, top - 0.5f),
        ImVec2(right + 0.5f, bottom + 0.5f),
        IM_COL32(0, 0, 0, 150), cornerRadius, 0, thickness + 0.5f
    );

    drawList->AddRect(
        ImVec2(left, top),
        ImVec2(right, bottom),
        color, cornerRadius, 0, thickness
    );

    const float accentSize = 8.0f;
    const float accentThickness = 2.0f;

    drawList->AddLine(ImVec2(left, top), ImVec2(left + accentSize, top), color, accentThickness);
    drawList->AddLine(ImVec2(left, top), ImVec2(left, top + accentSize), color, accentThickness);

    drawList->AddLine(ImVec2(right, top), ImVec2(right - accentSize, top), color, accentThickness);
    drawList->AddLine(ImVec2(right, top), ImVec2(right, top + accentSize), color, accentThickness);

    drawList->AddLine(ImVec2(left, bottom), ImVec2(left + accentSize, bottom), color, accentThickness);
    drawList->AddLine(ImVec2(left, bottom), ImVec2(left, bottom - accentSize), color, accentThickness);

    drawList->AddLine(ImVec2(right, bottom), ImVec2(right - accentSize, bottom), color, accentThickness);
    drawList->AddLine(ImVec2(right, bottom), ImVec2(right, bottom - accentSize), color, accentThickness);
}

void Renderer::DrawModernHealthBar(float boxLeft, float boxTop, float boxBottom, float boxWidth, int health, int armor) {
    if (health <= 0) return;

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    const float barWidth = 3.0f;
    const float barHeight = boxBottom - boxTop - 6.0f;
    const float barLeft = boxLeft - barWidth - 4.0f;
    const float barTop = boxTop + 3.0f;
    const float barBottom = boxBottom - 3.0f;

    const float cornerRadius = 1.5f;
    drawList->AddRectFilled(
        ImVec2(barLeft - 1, barTop - 1),
        ImVec2(barLeft + barWidth + 1, barBottom + 1),
        IM_COL32(0, 0, 0, 120), cornerRadius
    );

    float healthPercent = clamp(health / 100.0f, 0.0f, 1.0f);
    float currentBarHeight = barHeight * healthPercent;

    ImU32 healthColorTop, healthColorBottom;
    if (healthPercent > 0.7f) {
        healthColorTop = IM_COL32(120, 255, 120, 255);
        healthColorBottom = IM_COL32(60, 200, 60, 255);
    }
    else if (healthPercent > 0.4f) {
        healthColorTop = IM_COL32(255, 255, 120, 255);
        healthColorBottom = IM_COL32(200, 180, 60, 255);
    }
    else {
        healthColorTop = IM_COL32(255, 120, 120, 255);
        healthColorBottom = IM_COL32(200, 60, 60, 255);
    }

    drawList->AddRectFilledMultiColor(
        ImVec2(barLeft, barBottom - currentBarHeight),
        ImVec2(barLeft + barWidth, barBottom),
        healthColorBottom, healthColorBottom, healthColorTop, healthColorTop
    );

    ImU32 glowColor = (healthColorTop & 0x00FFFFFF) | 0x40000000;
    drawList->AddRect(
        ImVec2(barLeft - 0.5f, barBottom - currentBarHeight - 0.5f),
        ImVec2(barLeft + barWidth + 0.5f, barBottom + 0.5f),
        glowColor, cornerRadius, 0, 1.0f
    );

    if (armor > 0) {
        const float armorBarLeft = boxLeft - barWidth - 8.0f;
        float armorPercent = clamp(armor / 100.0f, 0.0f, 1.0f);
        float currentArmorHeight = barHeight * armorPercent;

        drawList->AddRectFilled(
            ImVec2(armorBarLeft - 1, barTop - 1),
            ImVec2(armorBarLeft + barWidth + 1, barBottom + 1),
            IM_COL32(0, 0, 0, 120), cornerRadius
        );

        ImU32 armorColorTop = IM_COL32(120, 180, 255, 255);
        ImU32 armorColorBottom = IM_COL32(60, 120, 200, 255);

        drawList->AddRectFilledMultiColor(
            ImVec2(armorBarLeft, barBottom - currentArmorHeight),
            ImVec2(armorBarLeft + barWidth, barBottom),
            armorColorBottom, armorColorBottom, armorColorTop, armorColorTop
        );
    }

    if (health < 100) {
        char healthText[16];
        sprintf_s(healthText, "%d", health);

        ImVec2 textSize = ImGui::CalcTextSize(healthText);
        ImVec2 textPos(barLeft - textSize.x - 2.0f, barBottom - currentBarHeight - textSize.y * 0.5f);

        drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 180), healthText);
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), healthText);
    }
}

void Renderer::DrawModernPlayerInfo(const Player& player, float boxLeft, float boxRight, float boxTop, float boxBottom) {
    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    float centerX = (boxLeft + boxRight) * 0.5f;
    float currentY = boxBottom + 4.0f;

    if (!player.name.empty() && player.distance < 100.0f) {
        ImVec2 nameSize = ImGui::CalcTextSize(player.name.c_str());

        drawList->AddRectFilled(
            ImVec2(centerX - nameSize.x * 0.5f - 4, currentY - 2),
            ImVec2(centerX + nameSize.x * 0.5f + 4, currentY + nameSize.y + 2),
            IM_COL32(0, 0, 0, 160), 3.0f
        );

        drawList->AddText(
            ImVec2(centerX - nameSize.x * 0.5f + 1, currentY + 1),
            IM_COL32(0, 0, 0, 200),
            player.name.c_str()
        );
        drawList->AddText(
            ImVec2(centerX - nameSize.x * 0.5f, currentY),
            IM_COL32(255, 255, 255, 255),
            player.name.c_str()
        );

        currentY += nameSize.y + 6;
    }

    char distanceText[32];
    sprintf_s(distanceText, "%.0fm", player.distance);

    ImVec2 distSize = ImGui::CalcTextSize(distanceText);

    drawList->AddRectFilled(
        ImVec2(centerX - distSize.x * 0.5f - 3, currentY - 1),
        ImVec2(centerX + distSize.x * 0.5f + 3, currentY + distSize.y + 1),
        IM_COL32(0, 0, 0, 140), 2.0f
    );

    drawList->AddText(
        ImVec2(centerX - distSize.x * 0.5f + 1, currentY + 1),
        IM_COL32(0, 0, 0, 180),
        distanceText
    );
    drawList->AddText(
        ImVec2(centerX - distSize.x * 0.5f, currentY),
        IM_COL32(200, 200, 200, 255),
        distanceText
    );
}

void Renderer::DrawModernSkeleton(const PlayerBones& bones, ImU32 color) {
    if (bones.screenPositions.empty()) return;

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    const std::vector<std::pair<BoneIndex, BoneIndex>> allConnections = {
        {HEAD, NECK}, {NECK, SPINE_UPPER}, {SPINE_UPPER, SPINE_LOWER}, {SPINE_LOWER, PELVIS},
        {SPINE_UPPER, LEFT_SHOULDER}, {LEFT_SHOULDER, LEFT_ELBOW}, {LEFT_ELBOW, LEFT_HAND},
        {SPINE_UPPER, RIGHT_SHOULDER}, {RIGHT_SHOULDER, RIGHT_ELBOW}, {RIGHT_ELBOW, RIGHT_HAND},
        {PELVIS, LEFT_HIP}, {LEFT_HIP, LEFT_KNEE}, {LEFT_KNEE, LEFT_FOOT},
        {PELVIS, RIGHT_HIP}, {RIGHT_HIP, RIGHT_KNEE}, {RIGHT_KNEE, RIGHT_FOOT}
    };

    ImU32 glowColor = (color & 0x00FFFFFF) | 0x60000000;

    for (const auto& connection : allConnections) {
        auto bone1It = bones.screenPositions.find(connection.first);
        auto bone2It = bones.screenPositions.find(connection.second);

        if (bone1It != bones.screenPositions.end() && bone2It != bones.screenPositions.end()) {
            const ScreenPos& pos1 = bone1It->second;
            const ScreenPos& pos2 = bone2It->second;

            if (pos1.valid && pos2.valid) {
                drawList->AddLine(ImVec2(pos1.x, pos1.y), ImVec2(pos2.x, pos2.y), glowColor, 3.0f);
                drawList->AddLine(ImVec2(pos1.x, pos1.y), ImVec2(pos2.x, pos2.y), color, 1.5f);
            }
        }
    }
}

void Renderer::DrawBox(const ScreenPos& head, const ScreenPos& foot, ImU32 color) {
    float boxHeight = std::abs(foot.y - head.y);
    float boxWidth = boxHeight * 0.4f;
    float boxLeft = head.x - (boxWidth * 0.5f);
    float boxRight = head.x + (boxWidth * 0.5f);

    DrawModernBox(boxLeft, head.y, boxRight, foot.y, color);
}

void Renderer::DrawHealthBar(const ScreenPos& pos, int health, int maxHealth) {
}

void Renderer::DrawSkeleton(const PlayerBones& bones, ImU32 color) {
    DrawModernSkeleton(bones, color);
}

void Renderer::DrawPlayerInfo(const Player& player) {
}
