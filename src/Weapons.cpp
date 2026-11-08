#include "Weapons.h"

#include <cstddef>

namespace sa {
namespace {

// Slot layout of the CWeapon[13] array on CPed (+0x5A0, 28 bytes each).
// The slot numbers follow the weapon slot list in the GTAMods wiki:
//
//   Slot 0  : No weapon (fist)
//   Slot 1  : Melee
//   Slot 2  : Handguns
//   Slot 3  : Shotguns
//   Slot 4  : Sub machine guns
//   Slot 5  : Machine guns
//   Slot 6  : Rifles
//   Slot 7  : Heavy weapons
//   Slot 8  : Projectiles
//   Slot 9  : Special 1
//   Slot 10 : Gifts
//   Slot 11 : Special 2
//   Slot 12 : Detonators
constexpr WeaponInfo kWeapons[] = {
    // Slot 0 - unarmed
    {0, 0, "Fist", 0},

    // Slot 1 - melee
    {1, 1, "Brass Knuckles", 0},
    {2, 1, "Golf Club", 0},
    {3, 1, "Nitestick", 0},
    {4, 1, "Knife", 0},
    {5, 1, "Baseball Bat", 0},
    {6, 1, "Shovel", 0},
    {7, 1, "Pool Cue", 0},
    {8, 1, "Katana", 0},
    {9, 1, "Chainsaw", 0},
    {15, 1, "Cane", 0},

    // Slot 2 - handguns
    {22, 2, "Pistol", 120},
    {23, 2, "Silenced Pistol", 120},
    {24, 2, "Desert Eagle", 120},

    // Slot 3 - shotguns
    {25, 3, "Shotgun", 100},
    {26, 3, "Sawn-Off Shotgun", 100},
    {27, 3, "SPAS-12", 100},

    // Slot 4 - sub machine guns
    {28, 4, "Micro Uzi", 200},
    {29, 4, "MP5", 200},
    {32, 4, "TEC-9", 200},

    // Slot 5 - machine guns
    {30, 5, "AK-47", 300},
    {31, 5, "M4", 300},

    // Slot 6 - rifles
    {33, 6, "Country Rifle", 300},
    {34, 6, "Sniper Rifle", 300},

    // Slot 7 - heavy weapons
    {35, 7, "Rocket Launcher", 300},
    {36, 7, "Heat Seeking RPG", 300},
    {37, 7, "Flamethrower", 300},
    {38, 7, "Minigun", 500},

    // Slot 8 - projectiles
    {16, 8, "Grenade", 30},
    {18, 8, "Molotov Cocktail", 30},
    {39, 8, "Remote Explosives", 30},
    {58, 8, "Flare", 30},

    // Slot 9 - special 1
    {42, 9, "Fire Extinguisher", 100},
    {43, 9, "Camera", 0},

    // Slot 10 - gifts
    {14, 10, "Flowers", 0},
    {46, 10, "Parachute", 0},

    // Slot 11 - special 2
    {44, 11, "Night Vision Goggles", 0},
    {45, 11, "Infrared Goggles", 0},

    // Slot 12 - detonators
    {40, 12, "Detonator", 0},
};

}  // namespace

const WeaponInfo* GetWeapons() { return kWeapons; }

int GetWeaponCount() { return static_cast<int>(sizeof(kWeapons) / sizeof(kWeapons[0])); }

const WeaponInfo* FindWeaponByType(uint8_t type) {
    for (const auto& weapon : kWeapons) {
        if (weapon.type == type) return &weapon;
    }
    return nullptr;
}

const WeaponInfo* FindWeaponByName(const std::string& name) {
    for (const auto& weapon : kWeapons) {
        if (name == weapon.name) return &weapon;
    }
    return nullptr;
}

int WeaponSlotForType(uint8_t weaponType) {
    const WeaponInfo* weapon = FindWeaponByType(weaponType);
    if (weapon == nullptr) return -1;
    return weapon->slot;
}

uint8_t WeaponTypeForSlot(int slot) {
    for (const auto& weapon : kWeapons) {
        if (weapon.slot == slot) return weapon.type;
    }
    return 0;
}

}  // namespace sa
