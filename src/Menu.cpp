#include "Menu.h"

#include <cmath>
#include <cstring>
#include <string>
#include <vector>

#include "imgui.h"

#include "Addresses.h"
#include "Cheats.h"
#include "Entities.h"
#include "Esp.h"
#include "Hook.h"
#include "Memory.h"
#include "PedModels.h"
#include "Spawner.h"
#include "VehicleModels.h"
#include "Weapons.h"

namespace sa {
namespace {

// The tabs of the menu.
enum MenuTab {
    kTabPlayer,
    kTabCheats,
    kTabEsp,
    kTabWeaponSpawner,
    kTabVehicleSpawner,
    kTabPedSpawner,
    kTabMisc,
    kTabCount,
};

const char* kTabNames[] = {
    "Player", "Cheats", "ESP", "Weapons", "Vehicles", "Peds", "Misc",
};

// --- Persistent UI state ----------------------------------------------------

int g_currentTab = kTabPlayer;

// Player tab
int g_moneyValue = 100000;
float g_healthValue = 100.0f;
float g_armorValue = 100.0f;
bool g_infiniteHealth = false;
bool g_infiniteArmor = false;

// Weapon spawner tab
int g_weaponIndex = 0;
int g_weaponAmmo = 120;
bool g_weaponSpawnSelected = false;

// Vehicle spawner tab
int g_vehicleIndex = 0;
bool g_vehicleSpawnSelected = false;
bool g_vehicleDeleteSpawned = false;

// Ped spawner tab
int g_pedIndex = 0;
int g_pedWeaponIndex = 0;
bool g_pedSpawnSelected = false;
bool g_pedArmed = true;
bool g_pedDeleteSpawned = false;

// Misc tab
int g_weatherIndex = 0;
int g_hour = 12;
int g_minute = 0;
float g_pedDensity = 1.0f;
float g_vehicleDensity = 1.0f;

// Cheat tab switches that map onto the game's cheat bytes.
struct CheatToggle {
    const char* label;
    uintptr_t address;
    bool* state;
};

// The state of every cheat switch, kept in sync with the game memory.
bool g_cheatWeaponSet1 = false;
bool g_cheatWeaponSet2 = false;
bool g_cheatWeaponSet3 = false;
bool g_cheatEveryoneArmed = false;
bool g_cheatInfiniteAmmo = false;
bool g_cheatInfiniteHealth = false;
bool g_cheatInfiniteOxygen = false;
bool g_cheatJetpack = false;
bool g_cheatParachute = false;
bool g_cheatMegaPunch = false;
bool g_cheatNeverHungry = false;
bool g_cheatPedsRiot = false;
bool g_cheatGangsControlStreets = false;
bool g_cheatGangMembersEverywhere = false;
bool g_cheatRecruitAnyone = false;
bool g_cheatCarsCanFly = false;
bool g_cheatBoatsCanFly = false;
bool g_cheatCarsDriveOnWater = false;
bool g_cheatTankMode = false;
bool g_cheatAllCarsHaveNitro = false;
bool g_cheatTrafficCheapCars = false;
bool g_cheatTrafficFastCars = false;
bool g_cheatAggressiveDrivers = false;
bool g_cheatWheelsOnly = false;
bool g_cheatPerfectHandling = false;
bool g_cheatHugeBunnyHop = false;
bool g_cheatMegaJump = false;
bool g_cheatPedsAttackWithRockets = false;
bool g_cheatElvisEverywhere = false;
bool g_cheatMaxMuscle = false;
bool g_cheatCJFat = false;
bool g_cheatCJSkinny = false;
bool g_cheatSlutMagnet = false;
bool g_cheatBountyOnHead = false;
bool g_cheatPedsGolfClub = false;
bool g_cheatLockWanted = false;
bool g_cheatSixStarWanted = false;
bool g_cheatFullAimWhileDriving = false;
bool g_cheatDecreasedTraffic = false;
bool g_cheatMaxRespect = false;
bool g_cheatMaxSexAppeal = false;
bool g_cheatMaxStamina = false;
bool g_cheatHitmanAllWeapons = false;

CheatToggle kCheatToggles[] = {
    {"Weapon Set 1", base::kCheatWeaponSet1, &g_cheatWeaponSet1},
    {"Weapon Set 2", base::kCheatWeaponSet2, &g_cheatWeaponSet2},
    {"Weapon Set 3", base::kCheatWeaponSet3, &g_cheatWeaponSet3},
    {"Everyone Is Armed", base::kCheatEveryoneArmed, &g_cheatEveryoneArmed},
    {"Infinite Health", base::kCheatInfiniteHealth, &g_cheatInfiniteHealth},
    {"Infinite Oxygen", base::kCheatInfiniteOxygen, &g_cheatInfiniteOxygen},
    {"Get Jetpack", base::kCheatGetJetpack, &g_cheatJetpack},
    {"Get Parachute", base::kCheatGetParachute, &g_cheatParachute},
    {"Mega Punch", base::kCheatMegaPunch, &g_cheatMegaPunch},
    {"Never Get Hungry", base::kCheatNeverHungry, &g_cheatNeverHungry},
    {"Peds Riot (Chaos)", base::kCheatPedsRiot, &g_cheatPedsRiot},
    {"Gangs Control The Streets", base::kCheatGangsControlStreets, &g_cheatGangsControlStreets},
    {"Gang Members Everywhere", base::kCheatGangMembersEverywhere, &g_cheatGangMembersEverywhere},
    {"Cars Can Fly", base::kCheatCarsCanFly, &g_cheatCarsCanFly},
    {"Boats Can Fly", base::kCheatBoatsCanFly, &g_cheatBoatsCanFly},
    {"Cars Drive On Water", base::kCheatCarsDriveOnWater, &g_cheatCarsDriveOnWater},
    {"Tank Mode", base::kCheatTankMode, &g_cheatTankMode},
    {"All Cars Have Nitro", base::kCheatAllCarsHaveNitro, &g_cheatAllCarsHaveNitro},
    {"Traffic Is Cheap Cars", base::kCheatTrafficCheapCars, &g_cheatTrafficCheapCars},
    {"Traffic Is Fast Cars", base::kCheatTrafficFastCars, &g_cheatTrafficFastCars},
    {"Aggressive Drivers", base::kCheatAggressiveDrivers, &g_cheatAggressiveDrivers},
    {"Wheels Only (Invisible Cars)", base::kCheatWheelsOnly, &g_cheatWheelsOnly},
    {"Perfect Handling", base::kCheatPerfectHandling, &g_cheatPerfectHandling},
    {"Huge Bunny Hop", base::kCheatHugeBunnyHop, &g_cheatHugeBunnyHop},
    {"Mega Jump", base::kCheatMegaJump, &g_cheatMegaJump},
    {"Peds Attack With Rockets", base::kCheatPedsAttackWithRockets, &g_cheatPedsAttackWithRockets},
    {"Elvis Everywhere", base::kCheatElvisEverywhere, &g_cheatElvisEverywhere},
    {"Max Muscle", base::kCheatMaxMuscle, &g_cheatMaxMuscle},
    {"CJ Is Fat", base::kCheatCJFat, &g_cheatCJFat},
    {"CJ Is Skinny", base::kCheatCJSkinny, &g_cheatCJSkinny},
    {"Slut Magnet", base::kCheatSlutMagnet, &g_cheatSlutMagnet},
    {"Bounty On Head", base::kCheatBountyOnHead, &g_cheatBountyOnHead},
    {"Peds Attack With Golf Club", base::kCheatPedsGolfClub, &g_cheatPedsGolfClub},
    {"Lock Wanted Level", base::kCheatLockWanted, &g_cheatLockWanted},
    {"Six Star Wanted Level", base::kCheatSixStarWanted, &g_cheatSixStarWanted},
    {"Full Aiming While Driving", base::kCheatFullAimWhileDriving, &g_cheatFullAimWhileDriving},
    {"Decreased Traffic", base::kCheatDecreasedTraffic, &g_cheatDecreasedTraffic},
    {"Max Respect", base::kCheatMaxRespect, &g_cheatMaxRespect},
    {"Max Sex Appeal", base::kCheatMaxSexAppeal, &g_cheatMaxSexAppeal},
    {"Max Stamina", base::kCheatMaxStamina, &g_cheatMaxStamina},
    {"Hitman In All Weapons", base::kCheatHitmanAllWeapons, &g_cheatHitmanAllWeapons},
};

// --- Helpers ----------------------------------------------------------------

void SectionHeader(const char* text) {
    ImGui::Separator();
    ImGui::TextUnformatted(text);
    ImGui::Separator();
}

// A checkbox that reads and writes a cheat byte in the game's memory.
void CheatCheckbox(const char* label, uintptr_t address, bool* state) {
    bool value = *state;
    if (ImGui::Checkbox(label, &value)) {
        *state = value;
        cheats::SetCheat(address, value);
    }
}

const char* kWeatherNames[] = {
    "Extrasunny (LA)", "Sunny (LA)", "Extrasunny (Smog)", "Sunny (Smog)", "Cloudy (LA)",
    "Sunny (SF)", "Extrasunny (SF)", "Cloudy (SF)", "Rainy (SF)", "Foggy (SF)",
    "Sunny (Vegas)", "Extrasunny (Vegas)", "Cloudy (Vegas)", "Extrasunny (Country)",
    "Sunny (Country)", "Cloudy (Country)", "Rainy (Country)", "Extrasunny (Desert)",
    "Sunny (Desert)", "Sandstorm (Desert)", "Underwater", "Extras Colours 1",
    "Extras Colours 2",
};

// --- Tabs -------------------------------------------------------------------

void DrawPlayerTab() {
    SectionHeader("Money");
    ImGui::InputInt("Amount", &g_moneyValue);
    if (ImGui::Button("Set Money")) cheats::SetMoney(g_moneyValue);
    ImGui::SameLine();
    if (ImGui::Button("Add Money")) cheats::AddMoney(g_moneyValue);
    ImGui::SameLine();
    if (ImGui::Button("Max Money")) cheats::SetMoney(999999999);

    SectionHeader("Health & Armour");
    ImGui::SliderFloat("Health", &g_healthValue, 0.0f, 100.0f);
    ImGui::SameLine();
    if (ImGui::Button("Set Health")) cheats::SetHealth(g_healthValue);

    ImGui::SliderFloat("Armour", &g_armorValue, 0.0f, 100.0f);
    ImGui::SameLine();
    if (ImGui::Button("Set Armour")) cheats::SetArmor(g_armorValue);

    ImGui::Checkbox("God Mode (no damage)", &g_infiniteHealth);
    if (g_infiniteHealth) {
        cheats::SetCheat(base::kCheatInfiniteHealth, true);
    }

    if (ImGui::Button("Full Health")) cheats::SetHealthMax();
    ImGui::SameLine();
    if (ImGui::Button("Full Armour")) cheats::SetArmor(100.0f);

    SectionHeader("Skills & Stats");
    if (ImGui::Button("Max All Skills")) {
        cheats::SetSkillsMax();
        cheats::ShowMessageInGame("Skills maxed");
    }
    ImGui::SameLine();
    if (ImGui::Button("Max Respect / Appeal")) {
        cheats::SetCheat(base::kCheatMaxRespect, true);
        cheats::SetCheat(base::kCheatMaxSexAppeal, true);
        cheats::SetCheat(base::kCheatMaxStamina, true);
    }

    SectionHeader("Wanted Level");
    if (ImGui::Button("Clear Wanted")) cheats::ClearWantedLevel();
    ImGui::SameLine();
    if (ImGui::Button("Set 3 Stars")) cheats::SetWantedLevel(3);
    ImGui::SameLine();
    if (ImGui::Button("Set 6 Stars")) cheats::SetWantedLevel(6);

    SectionHeader("Vehicle");
    if (ImGui::Button("Repair Vehicle")) cheats::RepairVehicle();
    ImGui::SameLine();
    if (ImGui::Button("Infinite Vehicle Health")) {
        cheats::SetCheat(base::kCheatAllCarsHaveNitro, false);
    }
    if (ImGui::Button("Nitro Refill")) {
        const uintptr_t vehicle = GetPlayerVehicle();
        if (vehicle != 0) {
            WriteFloat(vehicle + kVehNitroCount, 1.0f);
            WriteU8(vehicle + kVehNitroAvailable, 10);
        }
    }

    SectionHeader("Teleport");
    if (ImGui::Button("Teleport To Road")) {
        // Not implemented without the marker data; use the map instead.
    }
    ImGui::TextDisabled("Use the Vehicles / Peds tabs to spawn entities near you.");
}

void DrawCheatsTab() {
    SectionHeader("Weapons & Combat");
    CheatCheckbox("Weapon Set 1", base::kCheatWeaponSet1, &g_cheatWeaponSet1);
    CheatCheckbox("Weapon Set 2", base::kCheatWeaponSet2, &g_cheatWeaponSet2);
    CheatCheckbox("Weapon Set 3", base::kCheatWeaponSet3, &g_cheatWeaponSet3);
    CheatCheckbox("Everyone Is Armed", base::kCheatEveryoneArmed, &g_cheatEveryoneArmed);
    CheatCheckbox("Hitman In All Weapons", base::kCheatHitmanAllWeapons, &g_cheatHitmanAllWeapons);
    CheatCheckbox("Infinite Ammo / No Reload", base::kCheatInfiniteAmmo, &g_cheatInfiniteAmmo);
    CheatCheckbox("Mega Punch", base::kCheatMegaPunch, &g_cheatMegaPunch);
    CheatCheckbox("Recruit Anyone (9mm)", base::kCheatRecruitAnyone9mm, &g_cheatRecruitAnyone);

    SectionHeader("Player");
    CheatCheckbox("Infinite Health", base::kCheatInfiniteHealth, &g_cheatInfiniteHealth);
    CheatCheckbox("Infinite Oxygen", base::kCheatInfiniteOxygen, &g_cheatInfiniteOxygen);
    CheatCheckbox("Never Get Hungry", base::kCheatNeverHungry, &g_cheatNeverHungry);
    CheatCheckbox("Get Jetpack", base::kCheatGetJetpack, &g_cheatJetpack);
    CheatCheckbox("Get Parachute", base::kCheatGetParachute, &g_cheatParachute);
    CheatCheckbox("Max Respect", base::kCheatMaxRespect, &g_cheatMaxRespect);
    CheatCheckbox("Max Sex Appeal", base::kCheatMaxSexAppeal, &g_cheatMaxSexAppeal);
    CheatCheckbox("Max Stamina", base::kCheatMaxStamina, &g_cheatMaxStamina);
    CheatCheckbox("CJ Is Fat", base::kCheatCJFat, &g_cheatCJFat);
    CheatCheckbox("CJ Is Skinny", base::kCheatCJSkinny, &g_cheatCJSkinny);
    CheatCheckbox("Slut Magnet", base::kCheatSlutMagnet, &g_cheatSlutMagnet);
    CheatCheckbox("Bounty On Head", base::kCheatBountyOnHead, &g_cheatBountyOnHead);

    SectionHeader("World");
    CheatCheckbox("Peds Riot (Chaos Mode)", base::kCheatPedsRiot, &g_cheatPedsRiot);
    CheatCheckbox("Gangs Control The Streets", base::kCheatGangsControlStreets, &g_cheatGangsControlStreets);
    CheatCheckbox("Gang Members Everywhere", base::kCheatGangMembersEverywhere, &g_cheatGangMembersEverywhere);
    CheatCheckbox("Peds Attack With Golf Club", base::kCheatPedsGolfClub, &g_cheatPedsGolfClub);
    CheatCheckbox("Peds Attack With Rockets", base::kCheatPedsAttackWithRockets, &g_cheatPedsAttackWithRockets);
    CheatCheckbox("Elvis Everywhere", base::kCheatElvisEverywhere, &g_cheatElvisEverywhere);

    SectionHeader("Traffic & Vehicles");
    CheatCheckbox("Cars Can Fly", base::kCheatCarsCanFly, &g_cheatCarsCanFly);
    CheatCheckbox("Boats Can Fly", base::kCheatBoatsCanFly, &g_cheatBoatsCanFly);
    CheatCheckbox("Cars Drive On Water", base::kCheatCarsDriveOnWater, &g_cheatCarsDriveOnWater);
    CheatCheckbox("Tank Mode", base::kCheatTankMode, &g_cheatTankMode);
    CheatCheckbox("All Cars Have Nitro", base::kCheatAllCarsHaveNitro, &g_cheatAllCarsHaveNitro);
    CheatCheckbox("Traffic Is Cheap Cars", base::kCheatTrafficCheapCars, &g_cheatTrafficCheapCars);
    CheatCheckbox("Traffic Is Fast Cars", base::kCheatTrafficFastCars, &g_cheatTrafficFastCars);
    CheatCheckbox("Aggressive Drivers", base::kCheatAggressiveDrivers, &g_cheatAggressiveDrivers);
    CheatCheckbox("Decreased Traffic", base::kCheatDecreasedTraffic, &g_cheatDecreasedTraffic);
    CheatCheckbox("Wheels Only (Invisible Cars)", base::kCheatWheelsOnly, &g_cheatWheelsOnly);
    CheatCheckbox("Perfect Handling", base::kCheatPerfectHandling, &g_cheatPerfectHandling);
    CheatCheckbox("Huge Bunny Hop", base::kCheatHugeBunnyHop, &g_cheatHugeBunnyHop);
    CheatCheckbox("Mega Jump", base::kCheatMegaJump, &g_cheatMegaJump);
    CheatCheckbox("Full Aiming While Driving", base::kCheatFullAimWhileDriving, &g_cheatFullAimWhileDriving);
    CheatCheckbox("Lock Wanted Level", base::kCheatLockWanted, &g_cheatLockWanted);
    CheatCheckbox("Six Star Wanted Level", base::kCheatSixStarWanted, &g_cheatSixStarWanted);

    SectionHeader("Quick Actions");
    if (ImGui::Button("Reset All Cheats")) {
        for (const CheatToggle& toggle : kCheatToggles) {
            cheats::SetCheat(toggle.address, false);
            *toggle.state = false;
        }
        cheats::ShowMessageInGame("Cheats reset");
    }
}

void DrawEspTab() {
    EspSettings& esp = GetEspSettings();

    ImGui::Checkbox("Enable ESP", &esp.enabled);

    SectionHeader("Targets");
    ImGui::Checkbox("Police / Military", &esp.showCops);
    ImGui::Checkbox("Gang Members", &esp.showGangMembers);
    ImGui::Checkbox("Civilians", &esp.showCivilians);
    ImGui::Checkbox("Vehicles", &esp.showVehicles);

    SectionHeader("Style");
    ImGui::Checkbox("Show Boxes", &esp.showBoxes);
    ImGui::Checkbox("Show Names", &esp.showNames);
    ImGui::Checkbox("Show Distance", &esp.showDistance);
    ImGui::Checkbox("Show Health Bars", &esp.showHealth);
    ImGui::Checkbox("Show Tracers", &esp.showTracers);
    ImGui::Checkbox("Through Walls", &esp.throughWalls);

    ImGui::SliderFloat("Max Distance", &esp.maxDistance, 10.0f, 500.0f, "%.0f m");

    SectionHeader("Colours");
    auto colorButton = [](const char* label, uint32_t* color) {
        float rgba[4] = {
            static_cast<float>((*color >> 16) & 0xFF) / 255.0f,
            static_cast<float>((*color >> 8) & 0xFF) / 255.0f,
            static_cast<float>(*color & 0xFF) / 255.0f,
            static_cast<float>((*color >> 24) & 0xFF) / 255.0f,
        };
        if (ImGui::ColorEdit4(label, rgba, ImGuiColorEditFlags_AlphaBar)) {
            const auto r = static_cast<uint32_t>(rgba[0] * 255.0f);
            const auto g = static_cast<uint32_t>(rgba[1] * 255.0f);
            const auto b = static_cast<uint32_t>(rgba[2] * 255.0f);
            const auto a = static_cast<uint32_t>(rgba[3] * 255.0f);
            *color = (a << 24) | (r << 16) | (g << 8) | b;
        }
    };
    colorButton("Police##c1", &esp.copColor);
    colorButton("Gangs##c2", &esp.gangColor);
    colorButton("Civilians##c3", &esp.civilianColor);
    colorButton("Vehicles##c4", &esp.vehicleColor);
}

void DrawWeaponSpawnerTab() {
    SectionHeader("Give Weapon To Player");

    const WeaponInfo* weapons = GetWeapons();
    const int weaponCount = GetWeaponCount();
    if (g_weaponIndex >= weaponCount) g_weaponIndex = 0;

    static const char* slotNames[] = {
        "Unarmed", "Melee", "Handguns", "Shotguns", "SMG", "Machine Guns",
        "Rifles", "Heavy", "Projectiles", "Special 1", "Gifts", "Special 2",
        "Detonators",
    };

    std::vector<const char*> names;
    for (int i = 0; i < weaponCount; ++i) names.push_back(weapons[i].name);

    ImGui::Combo("Weapon", &g_weaponIndex, names.data(), weaponCount);
    ImGui::InputInt("Ammo", &g_weaponAmmo);
    if (g_weaponAmmo < 0) g_weaponAmmo = 0;

    const WeaponInfo& selected = weapons[g_weaponIndex];
    ImGui::Text("Type: %d   Slot: %d (%s)", selected.type, selected.slot,
                selected.slot < 13 ? slotNames[selected.slot] : "?");

    if (ImGui::Button("Give Weapon")) {
        const uintptr_t ped = GetPlayerPed();
        if (ped != 0 && GiveWeapon(ped, selected.type, static_cast<uint32_t>(g_weaponAmmo))) {
            cheats::ShowMessageInGame((std::string("Gave ") + selected.name).c_str());
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Give All Weapons")) {
        const uintptr_t ped = GetPlayerPed();
        if (ped != 0) {
            // Remember the slot the player had equipped: GiveWeapon() selects
            // the slot it fills, so without this the last weapon in the list
            // would end up in their hands.
            const uint8_t originalSlot = ReadU8(ped + kPedCurrentWeaponSlot);
            for (int i = 0; i < weaponCount; ++i) {
                if (weapons[i].defaultAmmo > 0) {
                    GiveWeapon(ped, weapons[i].type, weapons[i].defaultAmmo);
                }
            }
            WriteU8(ped + kPedCurrentWeaponSlot, originalSlot);
            cheats::ShowMessageInGame("All weapons given");
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove Weapons")) {
        const uintptr_t ped = GetPlayerPed();
        if (ped != 0) {
            for (int slot = 0; slot < kNumWeaponSlots; ++slot) {
                WriteU32(ped + kPedWeapons + slot * 0x1C + kWeaponType, 0);
            }
            WriteU8(ped + kPedCurrentWeaponSlot, 0);
        }
    }

    SectionHeader("Weapon Cheat Sets");
    ImGui::TextWrapped(
        "The cheat sets from the original game give you a full loadout of a "
        "specific weapon family.");
    if (ImGui::Button("Weapon Set 1")) {
        cheats::SetCheat(base::kCheatWeaponSet1, true);
        cheats::SetCheat(base::kCheatWeaponSet1, false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Weapon Set 2")) {
        cheats::SetCheat(base::kCheatWeaponSet2, true);
        cheats::SetCheat(base::kCheatWeaponSet2, false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Weapon Set 3")) {
        cheats::SetCheat(base::kCheatWeaponSet3, true);
        cheats::SetCheat(base::kCheatWeaponSet3, false);
    }
}

void DrawVehicleSpawnerTab() {
    SectionHeader("Spawn Vehicle");

    // Browse the full vehicles.ide table by category. The category strings
    // come from the static table so they stay valid for the lifetime of the
    // process - never build the combo items from temporary std::strings.
    static const char* kCategoryOrder[] = {
        "Car", "Bike", "Plane", "Boat", "Industrial", "Offroad", "RC",
        "Helicopter", "Train",
    };
    const int categoryCount =
        static_cast<int>(sizeof(kCategoryOrder) / sizeof(kCategoryOrder[0]));

    static int categoryIndex = 0;
    if (categoryIndex >= categoryCount) categoryIndex = 0;

    // Build the item list for the selected category once per frame. The
    // strings are owned by the static vehicle table.
    std::vector<const VehicleModel*> visible;
    const char* wanted = kCategoryOrder[categoryIndex];
    const VehicleModel* all = GetVehicleModels();
    const int allCount = GetVehicleModelCount();
    for (int i = 0; i < allCount; ++i) {
        if (std::strcmp(all[i].category, wanted) == 0) visible.push_back(&all[i]);
    }

    if (visible.empty()) {
        ImGui::TextDisabled("No vehicles in this category.");
    } else {
        if (g_vehicleIndex >= static_cast<int>(visible.size())) g_vehicleIndex = 0;

        std::vector<const char*> names;
        names.reserve(visible.size());
        for (const VehicleModel* model : visible) names.push_back(model->name);

        ImGui::Combo("Category", &categoryIndex, kCategoryOrder, categoryCount);
        ImGui::Combo("Vehicle", &g_vehicleIndex, names.data(),
                     static_cast<int>(names.size()));
        if (g_vehicleIndex >= static_cast<int>(visible.size())) g_vehicleIndex = 0;

        const uint16_t modelId = visible[g_vehicleIndex]->id;
        ImGui::Text("vehicles.ide id: %d  (%s)", modelId,
                    VehicleModelName(modelId).c_str());

        if (ImGui::Button("Spawn In Front Of Me")) {
            if (SpawnVehicle(modelId, SpawnOffset::InFront) != 0) {
                cheats::ShowMessageInGame(
                    (std::string("Spawned ") + VehicleModelName(modelId)).c_str());
            } else {
                cheats::ShowMessageInGame("Failed to spawn vehicle");
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Spawn On Player")) {
            SpawnVehicle(modelId, SpawnOffset::AtPlayer);
        }
    }

    SectionHeader("Custom Model Id");
    static int customModelId = 411;
    ImGui::InputInt("Model Id", &customModelId);
    if (ImGui::Button("Spawn Custom Model")) {
        if (customModelId >= 0 && customModelId <= 65535) {
            SpawnVehicle(static_cast<uint16_t>(customModelId), SpawnOffset::InFront);
        }
    }

    SectionHeader("Manage Spawned Vehicles");
    if (ImGui::Button("Delete Spawned Vehicles")) {
        const int count = DeleteSpawnedEntities();
        cheats::ShowMessageInGame((std::string("Deleted ") + std::to_string(count) +
                                   " entities").c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete All Vehicles")) {
        int deleted = 0;
        for (const Entity& vehicle : GetVehicles()) {
            if (DeleteEntity(vehicle.address, true)) ++deleted;
        }
        cheats::ShowMessageInGame((std::string("Deleted ") + std::to_string(deleted) +
                                   " vehicles").c_str());
    }

    SectionHeader("Vehicle Density");
    ImGui::SliderFloat("Traffic Density", &g_vehicleDensity, 0.0f, 5.0f, "%.2fx");
    if (ImGui::Button("Apply Density")) cheats::SetVehicleDensity(g_vehicleDensity);
}

void DrawPedSpawnerTab() {
    SectionHeader("Spawn Ped");

    // Browse the stock ped models by group. The group strings come from the
    // static ped table, so they stay valid for the lifetime of the process.
    static const char* kPedGroupOrder[] = {
        "Civilian", "Gang", "Police", "Story", "Unused",
    };
    const int groupCount =
        static_cast<int>(sizeof(kPedGroupOrder) / sizeof(kPedGroupOrder[0]));

    static int groupIndex = 0;
    if (groupIndex >= groupCount) groupIndex = 0;

    std::vector<const PedModel*> visible;
    const char* wanted = kPedGroupOrder[groupIndex];
    const PedModel* all = GetPedModels();
    const int allCount = GetPedModelCount();
    for (int i = 0; i < allCount; ++i) {
        if (std::strcmp(all[i].group, wanted) == 0) visible.push_back(&all[i]);
    }

    if (visible.empty()) {
        ImGui::TextDisabled("No peds in this group.");
    } else {
        if (g_pedIndex >= static_cast<int>(visible.size())) g_pedIndex = 0;

        std::vector<const char*> names;
        names.reserve(visible.size());
        for (const PedModel* model : visible) names.push_back(model->name);

        ImGui::Combo("Group", &groupIndex, kPedGroupOrder, groupCount);
        ImGui::Combo("Ped", &g_pedIndex, names.data(),
                     static_cast<int>(names.size()));
        if (g_pedIndex >= static_cast<int>(visible.size())) g_pedIndex = 0;

        const uint16_t modelId = visible[g_pedIndex]->id;
        ImGui::Text("peds.ide id: %d  (%s)", modelId, PedModelName(modelId).c_str());

        ImGui::Checkbox("Spawn Armed", &g_pedArmed);
        if (g_pedArmed) {
            const WeaponInfo* weapons = GetWeapons();
            const int weaponCount = GetWeaponCount();
            if (g_pedWeaponIndex >= weaponCount) g_pedWeaponIndex = 0;

            std::vector<const char*> weaponNames;
            for (int i = 0; i < weaponCount; ++i) {
                weaponNames.push_back(weapons[i].name);
            }
            ImGui::Combo("Weapon", &g_pedWeaponIndex, weaponNames.data(),
                         weaponCount);
        }

        if (ImGui::Button("Spawn Next To Me")) {
            uintptr_t ped = 0;
            if (g_pedArmed) {
                const WeaponInfo* weapons = GetWeapons();
                ped = SpawnArmedPed(modelId, weapons[g_pedWeaponIndex].type,
                                    weapons[g_pedWeaponIndex].defaultAmmo);
            } else {
                ped = SpawnPed(modelId, SpawnOffset::ToTheRight);
            }
            cheats::ShowMessageInGame(ped != 0 ? "Ped spawned" : "Failed to spawn ped");
        }
    }

    SectionHeader("Manage Spawned Peds");
    if (ImGui::Button("Delete Spawned Peds")) {
        const int count = DeleteSpawnedEntities();
        cheats::ShowMessageInGame((std::string("Deleted ") + std::to_string(count) +
                                   " entities").c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("Kill All Peds")) {
        int killed = 0;
        for (const Entity& ped : GetPeds()) {
            WriteFloat(ped.address + kPedHealth, 0.0f);
            WriteU32(ped.address + kPedState, 55);  // wasted
            ++killed;
        }
        cheats::ShowMessageInGame((std::string("Killed ") + std::to_string(killed) +
                                   " peds").c_str());
    }

    SectionHeader("Ped Density");
    ImGui::SliderFloat("Ped Density", &g_pedDensity, 0.0f, 5.0f, "%.2fx");
    if (ImGui::Button("Apply Density")) cheats::SetPedDensity(g_pedDensity);
}

void DrawMiscTab() {
    SectionHeader("Weather");
    ImGui::Combo("Weather", &g_weatherIndex, kWeatherNames,
                 static_cast<int>(sizeof(kWeatherNames) / sizeof(kWeatherNames[0])));
    if (ImGui::Button("Set Weather")) {
        cheats::SetWeather(g_weatherIndex);
        cheats::ShowMessageInGame("Weather changed");
    }

    SectionHeader("Time");
    ImGui::SliderInt("Hour", &g_hour, 0, 23);
    ImGui::SliderInt("Minute", &g_minute, 0, 59);
    if (ImGui::Button("Set Time")) cheats::SetTime(g_hour, g_minute);

    static bool freezeTime = false;
    ImGui::Checkbox("Freeze Time", &freezeTime);
    if (freezeTime) cheats::FreezeTime(true);

    SectionHeader("Display");
    static bool hudEnabled = true;
    if (ImGui::Checkbox("Show HUD", &hudEnabled)) {
        WriteU8(base::kHudMode, hudEnabled ? 1 : 0);
    }
    static bool radarEnabled = true;
    if (ImGui::Checkbox("Show Radar", &radarEnabled)) {
        WriteU8(base::kRadarMode, radarEnabled ? 0 : 2);
    }
    static bool subtitlesEnabled = true;
    if (ImGui::Checkbox("Subtitles", &subtitlesEnabled)) {
        WriteU8(base::kSubtitles, subtitlesEnabled ? 1 : 0);
    }
    static float brightness = 0.5f;
    ImGui::SliderFloat("Brightness", &brightness, 0.0f, 1.0f, "%.2f");
    if (ImGui::Button("Apply Brightness")) {
        WriteU32(base::kBrightness, static_cast<uint32_t>(brightness * 255.0f));
    }

    SectionHeader("Game Speed");
    static float gameSpeed = 1.0f;
    ImGui::SliderFloat("Game Speed", &gameSpeed, 0.0f, 2.0f, "%.2fx");
    if (ImGui::Button("Apply Game Speed")) {
        WriteFloat(base::kGameSpeed, gameSpeed);
    }

    SectionHeader("About");
    ImGui::Text("GTA: San Andreas ImGui Cheat Menu");
    ImGui::Text("Press INSERT to toggle the menu.");
    ImGui::TextDisabled("Memory addresses target GTA_SA.EXE v1.0.");

    if (ImGui::Button("Close Menu")) SetMenuOpen(false);
}

}  // namespace

void DrawMainMenu() {
    ImGui::SetNextWindowSize(ImVec2(640.0f, 560.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(80.0f, 80.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin("GTA San Andreas - Cheat Menu",
                 nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

    if (ImGui::BeginTabBar("##tabs")) {
        if (ImGui::BeginTabItem(kTabNames[kTabPlayer])) {
            DrawPlayerTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(kTabNames[kTabCheats])) {
            DrawCheatsTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(kTabNames[kTabEsp])) {
            DrawEspTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(kTabNames[kTabWeaponSpawner])) {
            DrawWeaponSpawnerTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(kTabNames[kTabVehicleSpawner])) {
            DrawVehicleSpawnerTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(kTabNames[kTabPedSpawner])) {
            DrawPedSpawnerTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(kTabNames[kTabMisc])) {
            DrawMiscTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

}  // namespace sa
