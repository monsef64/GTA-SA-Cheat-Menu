// Cheats.h - The cheat functions that the menu calls into.

#pragma once

#include <cstdint>

namespace sa {
namespace cheats {

// Player related -----------------------------------------------------------
void SetMoney(int amount);
void AddMoney(int amount);
void SetHealth(float value);
void SetArmor(float value);
void SetWantedLevel(int level);
void ClearWantedLevel();
void TeleportToMarker();
void SetHealthMax();
void SetSkillsMax();
void SetVehicleHealth();
void RepairVehicle();

// Cheat switch helpers -----------------------------------------------------
void SetCheat(uintptr_t address, bool enabled);
void ToggleCheat(uintptr_t address);

// Weather ------------------------------------------------------------------
void SetWeather(int weatherId);

// Time ---------------------------------------------------------------------
void SetTime(int hour, int minute);
void FreezeTime(bool freeze);

// Density ------------------------------------------------------------------
void SetPedDensity(float multiplier);
void SetVehicleDensity(float multiplier);

// Misc ---------------------------------------------------------------------
void ShowMessageInGame(const char* text);

}  // namespace cheats
}  // namespace sa
