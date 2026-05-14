#pragma once
#include <Windows.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include "Offset.hpp"

struct Vector3 {
    float x, y, z;
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    float length() const {
        return sqrtf(x * x + y * y + z * z);
    }

    float length2d() const {
        return sqrtf(x * x + y * y);
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
};

struct ViewMatrix {
    float matrix[4][4];
};

struct ScreenPos {
    float x, y;
    bool valid;
    ScreenPos() : x(0), y(0), valid(false) {}
    ScreenPos(float x, float y, bool valid = true) : x(x), y(y), valid(valid) {}
};

enum BoneIndex {
    HEAD = 6,
    NECK = 5,
    SPINE_UPPER = 4,
    SPINE_LOWER = 2,
    PELVIS = 0,
    LEFT_SHOULDER = 8,
    LEFT_ELBOW = 9,
    LEFT_HAND = 10,
    RIGHT_SHOULDER = 13,
    RIGHT_ELBOW = 14,
    RIGHT_HAND = 15,
    LEFT_HIP = 22,
    LEFT_KNEE = 23,
    LEFT_FOOT = 24,
    RIGHT_HIP = 25,
    RIGHT_KNEE = 26,
    RIGHT_FOOT = 27
};

struct PlayerBones {
    std::unordered_map<int, Vector3> positions;
    std::unordered_map<int, ScreenPos> screenPositions;

    void clear() {
        positions.clear();
        screenPositions.clear();
    }
};

struct Player {
    uintptr_t entity = 0;
    uintptr_t playerPawn = 0;
    uintptr_t gameSceneNode = 0;
    uintptr_t boneArray = 0;

    Vector3 position;
    Vector3 headPos;
    ScreenPos screenPos;
    ScreenPos headScreenPos;

    PlayerBones bones;

    int health = 0;
    int armor = 0;
    int team = 0;
    float distance = 0.0f;

    std::string name;
    bool isValid = false;
    bool isVisible = false;

    void clear() {
        bones.clear();
        isValid = false;
    }
};

class EntitySystem {
private:
    std::vector<Player> players;
    std::mutex playersMutex;

    uintptr_t localPlayerController = 0;
    uintptr_t localPlayerPawn = 0;
    uintptr_t entityList = 0;

    Vector3 localPosition;
    int localTeam = 0;
    ViewMatrix viewMatrix;

    bool isRunning = false;
    uintptr_t processId = 0;
    uintptr_t processCr3 = 0;

public:
    void Initialize(uintptr_t pid);
    void Update();
    void Shutdown();

    std::vector<Player> GetPlayers();
    Vector3 GetLocalPosition() const { return localPosition; }

    ScreenPos WorldToScreen(const Vector3& worldPos) const;

private:
    bool ReadPlayer(int index, Player& player);
    void ReadPlayerBones(Player& player);
    Vector3 ReadBonePosition(uintptr_t boneArray, int boneIndex);
};

extern EntitySystem g_EntitySystem;
