#pragma once

#include <cstddef>

namespace cs2_dumper {
    namespace offsets {
        namespace client_dll {
            constexpr std::ptrdiff_t dwCSGOInput = 0x1E2B4F0;
            constexpr std::ptrdiff_t dwEntityList = 0x1D042D8;
            constexpr std::ptrdiff_t dwGameEntitySystem = 0x1FA51C8;
            constexpr std::ptrdiff_t dwGameEntitySystem_highestEntityIndex = 0x20F0;
            constexpr std::ptrdiff_t dwGameRules = 0x1E20338;
            constexpr std::ptrdiff_t dwGlobalVars = 0x1BD4FA0;
            constexpr std::ptrdiff_t dwGlowManager = 0x1E1D1D8;
            constexpr std::ptrdiff_t dwLocalPlayerController = 0x1E0D3E8;
            constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x1BDFD10;
            constexpr std::ptrdiff_t dwPlantedC4 = 0x1E25A58;
            constexpr std::ptrdiff_t dwPrediction = 0x1BDFC40;
            constexpr std::ptrdiff_t dwSensitivity = 0x1E1DC38;
            constexpr std::ptrdiff_t dwSensitivity_sensitivity = 0x48;
            constexpr std::ptrdiff_t dwViewAngles = 0x1E2BBA0;
            constexpr std::ptrdiff_t dwViewMatrix = 0x1E21090;
            constexpr std::ptrdiff_t dwViewRender = 0x1E21EB0;
            constexpr std::ptrdiff_t dwWeaponC4 = 0x1DBE948;
        }
    }
}