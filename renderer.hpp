#pragma once
#include "entities.hpp"
#include "Imgui/imgui.h"

class Renderer {
public:
    void RenderESP();

private:
    void DrawModernBox(float left, float top, float right, float bottom, ImU32 color);
    void DrawModernHealthBar(float boxLeft, float boxTop, float boxBottom, float boxWidth, int health, int armor);
    void DrawModernPlayerInfo(const Player& player, float boxLeft, float boxRight, float boxTop, float boxBottom);
    void DrawModernSkeleton(const PlayerBones& bones, ImU32 color);

    void DrawBox(const ScreenPos& head, const ScreenPos& foot, ImU32 color);
    void DrawHealthBar(const ScreenPos& pos, int health, int maxHealth = 100);
    void DrawSkeleton(const PlayerBones& bones, ImU32 color);
    void DrawPlayerInfo(const Player& player);
};

extern Renderer g_Renderer;
