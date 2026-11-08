// Weapons.h - Weapon definitions and the helpers to give weapons to peds.
//
// The weapon type values match the eWeaponType enum used by the game and the
// slot index is the position of the weapon inside the CWeapon[13] array that
// starts at CPed + 0x5A0.

#pragma once

#include <cstdint>
#include <string>

namespace sa {

struct WeaponInfo {
    uint8_t type;         // eWeaponType value
    int slot;             // index inside the CWeapon[13] array
    const char* name;
    uint32_t defaultAmmo; // ammo used when spawned
};

// Every weapon that can be given to the player or to a spawned ped.
const WeaponInfo* GetWeapons();
int GetWeaponCount();

// Looks a weapon up by its eWeaponType value.
const WeaponInfo* FindWeaponByType(uint8_t type);

// Looks a weapon up by its display name.
const WeaponInfo* FindWeaponByName(const std::string& name);

// Maps a weapon type to its slot index, or -1 when the weapon has no slot.
int WeaponSlotForType(uint8_t weaponType);

// Maps a slot index back to the weapon type it holds.
uint8_t WeaponTypeForSlot(int slot);

}  // namespace sa
