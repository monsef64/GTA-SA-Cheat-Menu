#include "Cheats.h"

#include <cstdio>
#include <cstring>
#include <string>

#include "Addresses.h"
#include "Entities.h"
#include "Memory.h"
#include "Spawner.h"

namespace sa {
namespace cheats {

void SetMoney(int amount) {
    if (amount < 0) amount = 0;
    WriteI32(base::kMoney, amount);
}

void AddMoney(int amount) {
    const int current = ReadI32(base::kMoney);
    int next = current + amount;
    if (next < 0) next = 0;
    WriteI32(base::kMoney, next);
}

void SetHealth(float value) {
    const uintptr_t ped = GetPlayerPed();
    if (ped == 0) return;
    WriteFloat(ped + kPedHealth, value);
    WriteFloat(ped + kPedMaxHealth, value);
}

void SetArmor(float value) {
    const uintptr_t ped = GetPlayerPed();
    if (ped == 0) return;
    WriteFloat(ped + kPedArmor, value);
}

void SetWantedLevel(int level) {
    if (level < 0) level = 0;
    if (level > 6) level = 6;
    WriteI32(base::kWantedLevel, level);
}

void ClearWantedLevel() {
    WriteI32(base::kWantedLevel, 0);
    // The cheat switch also resets the internal wanted counter.
    SetCheat(base::kCheatClearWanted, true);
    SetCheat(base::kCheatClearWanted, false);
}

void SetHealthMax() {
    const uintptr_t ped = GetPlayerPed();
    if (ped == 0) return;
    WriteFloat(ped + kPedHealth, 100.0f);
    WriteFloat(ped + kPedMaxHealth, 100.0f);
}

void SetSkillsMax() {
    WriteU32(base::kSkillDriving, 1000);
    WriteU32(base::kSkillFlying, 1000);
    WriteU32(base::kSkillBike, 1000);
    WriteU32(base::kSkillCycling, 1000);
    WriteU32(base::kSkillGambling, 1000);
    WriteU32(base::kStatEnergy, 1000);
    WriteU32(base::kStatLungCapacity, 1000);
    WriteFloat(base::kStatStamina, 1000.0f);
    WriteFloat(base::kStatMuscle, 1000.0f);
    WriteFloat(base::kStatHealth, 1000.0f);
    WriteFloat(base::kStatSexAppeal, 1000.0f);
    WriteFloat(base::kStatRespect, 1000.0f);
}

void SetVehicleHealth() {
    const uintptr_t vehicle = GetPlayerVehicle();
    if (vehicle == 0) return;
    WriteFloat(vehicle + kVehHealth, 1000.0f);
    WriteFloat(vehicle + kVehBurnTimer, 0.0f);
}

void RepairVehicle() {
    const uintptr_t vehicle = GetPlayerVehicle();
    if (vehicle == 0) return;
    WriteFloat(vehicle + kVehHealth, 1000.0f);
    WriteFloat(vehicle + kVehBurnTimer, 0.0f);
    WriteU8(vehicle + kVehEngineState, 16);  // engine running
    // Restore the tyres.
    for (uintptr_t offset : {0x5A5, 0x5A6, 0x5A7, 0x5A8, 0x65C, 0x65D}) {
        WriteU8(vehicle + offset, 0);
    }
}

void SetCheat(uintptr_t address, bool enabled) {
    WriteU8(address, enabled ? 1 : 0);
}

void ToggleCheat(uintptr_t address) {
    const uint8_t current = ReadU8(address);
    WriteU8(address, current != 0 ? 0 : 1);
}

void SetWeather(int weatherId) {
    if (weatherId < 0) weatherId = 0;
    if (weatherId > 45) weatherId = 45;
    WriteU16(base::kCurrentWeather, static_cast<uint16_t>(weatherId));
    WriteU16(base::kUpcomingWeather, static_cast<uint16_t>(weatherId));
    WriteU16(base::kWeatherLock, static_cast<uint16_t>(weatherId));
}

void SetTime(int hour, int minute) {
    if (hour < 0) hour = 0;
    if (hour > 23) hour = 23;
    if (minute < 0) minute = 0;
    if (minute > 59) minute = 59;
    WriteU8(base::kCurrentHour, static_cast<uint8_t>(hour));
    WriteU8(base::kCurrentMinute, static_cast<uint8_t>(minute));
}

void FreezeTime(bool freeze) {
    // The game stores the "stop game clock" cheat switch at 0x969168.
    SetCheat(base::kCheatStopClock, freeze);
}

void SetPedDensity(float multiplier) {
    if (multiplier < 0.0f) multiplier = 0.0f;
    if (multiplier > 10.0f) multiplier = 10.0f;
    WriteFloat(base::kPedDensity, multiplier);
}

void SetVehicleDensity(float multiplier) {
    if (multiplier < 0.0f) multiplier = 0.0f;
    if (multiplier > 10.0f) multiplier = 10.0f;
    WriteFloat(base::kVehicleDensity, multiplier);
}

void ShowMessageInGame(const char* text) {
    if (text == nullptr) return;
    ShowMessage(std::string(text));
}

}  // namespace cheats
}  // namespace sa
