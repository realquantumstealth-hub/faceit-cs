#include "entities.hpp"
#include "system.h"
#include "hypercall/hypercall.h"
#include <iostream>

extern uintptr_t clientModule;

EntitySystem g_EntitySystem;

void EntitySystem::Initialize(uintptr_t pid) {
    this->processId = pid;
    this->processCr3 = sys::get_process_cr3(pid);
    isRunning = true;
    std::cout << "[+] Entity system initialized with PID: " << pid << std::endl;
    std::cout << "[+] Process CR3: 0x" << std::hex << this->processCr3 << std::dec << std::endl;
}

void EntitySystem::Update() {

    if (!isRunning || !this->processId) {
        return;
    }

    std::lock_guard<std::mutex> lock(playersMutex);

    hypercall::read_guest_virtual_memory(&localPlayerController, clientModule + cs2_dumper::offsets::client_dll::dwLocalPlayerController, this->processCr3, sizeof(localPlayerController));

    if (!localPlayerController) {
        return;
    }

    uint32_t localPawnHandle = 0;
    hypercall::read_guest_virtual_memory(&localPawnHandle, localPlayerController + 0x8FC, this->processCr3, sizeof(localPawnHandle));

    hypercall::read_guest_virtual_memory(&entityList, clientModule + cs2_dumper::offsets::client_dll::dwEntityList, this->processCr3, sizeof(entityList));

    uintptr_t localListEntry = 0;
    hypercall::read_guest_virtual_memory(&localListEntry, entityList + 0x8 * ((localPawnHandle & 0x7FFF) >> 9) + 16, this->processCr3, sizeof(localListEntry));

    hypercall::read_guest_virtual_memory(&localPlayerPawn, localListEntry + 120 * (localPawnHandle & 0x1FF), this->processCr3, sizeof(localPlayerPawn));

    if (!localPlayerPawn) {
        return;
    }

    hypercall::read_guest_virtual_memory(&localPosition, localPlayerPawn + 0x15B8, this->processCr3, sizeof(localPosition));

    hypercall::read_guest_virtual_memory(&localTeam, localPlayerController + 0x3EB, this->processCr3, sizeof(localTeam));

    hypercall::read_guest_virtual_memory(&viewMatrix, clientModule + cs2_dumper::offsets::client_dll::dwViewMatrix, this->processCr3, sizeof(viewMatrix));

    std::vector<Player> tempPlayers;
    tempPlayers.reserve(32);

    for (int i = 1; i <= 64; ++i) {
        Player player;
        if (ReadPlayer(i, player)) {
            Vector3 delta = player.position - localPosition;
            player.distance = delta.length();
            ReadPlayerBones(player);
            tempPlayers.push_back(std::move(player));
        }
    }

    players = std::move(tempPlayers);
}

bool EntitySystem::ReadPlayer(int index, Player& player) {
    uintptr_t listEntry = 0;
    hypercall::read_guest_virtual_memory(&listEntry, entityList + (8 * (index & 0x7FFF) >> 9) + 16, this->processCr3, sizeof(listEntry));
    if (!listEntry) return false;

    hypercall::read_guest_virtual_memory(&player.entity, listEntry + 120 * (index & 0x1FF), this->processCr3, sizeof(player.entity));
    if (!player.entity) return false;

    hypercall::read_guest_virtual_memory(&player.team, player.entity + 0x3EB, this->processCr3, sizeof(player.team));
    if (player.team < 1 || player.team > 3) return false;

    uint32_t pawnHandle = 0;
    hypercall::read_guest_virtual_memory(&pawnHandle, player.entity + 0x8FC, this->processCr3, sizeof(pawnHandle));
    if (!pawnHandle) return false;

    uintptr_t pawnListEntry = 0;
    hypercall::read_guest_virtual_memory(&pawnListEntry, entityList + 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 16, this->processCr3, sizeof(pawnListEntry));
    if (!pawnListEntry) return false;

    hypercall::read_guest_virtual_memory(&player.playerPawn, pawnListEntry + 120 * (pawnHandle & 0x1FF), this->processCr3, sizeof(player.playerPawn));
    if (!player.playerPawn) return false;

    hypercall::read_guest_virtual_memory(&player.health, player.playerPawn + 0x34C, this->processCr3, sizeof(player.health));
    if (player.health <= 0 || player.health > 100) return false;

    hypercall::read_guest_virtual_memory(&player.armor, player.playerPawn + 0x2764, this->processCr3, sizeof(player.armor));

    hypercall::read_guest_virtual_memory(&player.position, player.playerPawn + 0x15B8, this->processCr3, sizeof(player.position));
    player.headPos = Vector3(player.position.x, player.position.y, player.position.z + 75.0f);

    if ((player.position - localPosition).length() < 200.0f) {
        char nameBuffer[64] = {};
        hypercall::read_guest_virtual_memory(nameBuffer, player.entity + 0x6E8, this->processCr3, 63);
        player.name = std::string(nameBuffer);
    }

    hypercall::read_guest_virtual_memory(&player.gameSceneNode, player.playerPawn + 0x330, this->processCr3, sizeof(player.gameSceneNode));
    if (player.gameSceneNode) {
        hypercall::read_guest_virtual_memory(&player.boneArray, player.gameSceneNode + 0x210, this->processCr3, sizeof(player.boneArray));
    }

    player.isValid = true;
    return true;
}

void EntitySystem::ReadPlayerBones(Player& player) {
    if (!player.boneArray) return;

    player.bones.clear();

    const std::vector<BoneIndex> allBones = {
        HEAD, NECK, SPINE_UPPER, SPINE_LOWER, PELVIS,
        LEFT_SHOULDER, LEFT_ELBOW, LEFT_HAND,
        RIGHT_SHOULDER, RIGHT_ELBOW, RIGHT_HAND,
        LEFT_HIP, LEFT_KNEE, LEFT_FOOT,
        RIGHT_HIP, RIGHT_KNEE, RIGHT_FOOT
    };

    for (BoneIndex bone : allBones) {
        Vector3 bonePos = ReadBonePosition(player.boneArray, bone);
        if (bonePos.x != 0 || bonePos.y != 0 || bonePos.z != 0) {
            player.bones.positions[bone] = bonePos;
        }
    }
}

ScreenPos EntitySystem::WorldToScreen(const Vector3& worldPos) const {
    float screenW = (viewMatrix.matrix[3][0] * worldPos.x) +
        (viewMatrix.matrix[3][1] * worldPos.y) +
        (viewMatrix.matrix[3][2] * worldPos.z) +
        viewMatrix.matrix[3][3];

    if (screenW < 0.001f) return ScreenPos(0, 0, false);

    float screenX = (viewMatrix.matrix[0][0] * worldPos.x) +
        (viewMatrix.matrix[0][1] * worldPos.y) +
        (viewMatrix.matrix[0][2] * worldPos.z) +
        viewMatrix.matrix[0][3];

    float screenY = (viewMatrix.matrix[1][0] * worldPos.x) +
        (viewMatrix.matrix[1][1] * worldPos.y) +
        (viewMatrix.matrix[1][2] * worldPos.z) +
        viewMatrix.matrix[1][3];

    float invW = 1.0f / screenW;
    float camX = GetSystemMetrics(SM_CXSCREEN) * 0.5f;
    float camY = GetSystemMetrics(SM_CYSCREEN) * 0.5f;

    float x = camX + (camX * screenX * invW);
    float y = camY - (camY * screenY * invW);

    bool valid = (x >= -100.0f && x <= GetSystemMetrics(SM_CXSCREEN) + 100.0f &&
        y >= -100.0f && y <= GetSystemMetrics(SM_CYSCREEN) + 100.0f);

    return ScreenPos(x, y, valid);
}

std::vector<Player> EntitySystem::GetPlayers() {
    std::lock_guard<std::mutex> lock(playersMutex);
    return players;
}

Vector3 EntitySystem::ReadBonePosition(uintptr_t boneArray, int boneIndex) {
    if (!boneArray) return Vector3(0, 0, 0);

    uintptr_t boneAddress = boneArray + (boneIndex * 32);
    Vector3 bone_pos;
    hypercall::read_guest_virtual_memory(&bone_pos, boneAddress, this->processCr3, sizeof(bone_pos));
    return bone_pos;
}

void EntitySystem::Shutdown() {
    isRunning = false;
    std::lock_guard<std::mutex> lock(playersMutex);
    players.clear();
    std::cout << "[+] Entity system shutdown" << std::endl;
}
