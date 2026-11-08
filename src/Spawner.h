// Spawner.h - Creates vehicles and peds inside the running game.
//
// The game exposes CVehicle::CreateCar (0x43A0B6) and the SCM opcode
// handlers. Rather than guessing at raw struct layouts we call the game's own
// factory functions, which keeps the spawned entities fully valid (they get
// proper collision models, tasks and AI).

#pragma once

#include <cstdint>

namespace sa {

// Where a spawned entity is placed relative to the player.
enum class SpawnOffset {
    AtPlayer,      // exactly on the player
    InFront,       // a few metres in front of the player
    ToTheRight,    // a few metres to the player's right
    SlightlyAbove, // above the player (useful for planes)
};

// Creates a vehicle of the given vehicles.ide id in front of the player.
// Returns the address of the new CVehicle, or 0 on failure.
uintptr_t SpawnVehicle(uint16_t modelId, SpawnOffset offset = SpawnOffset::InFront);

// Creates a ped of the given ped model type next to the player.
// Returns the address of the new CPed, or 0 on failure.
uintptr_t SpawnPed(uint32_t pedModelType, SpawnOffset offset = SpawnOffset::InFront);

// Creates a ped and gives it a weapon.
uintptr_t SpawnArmedPed(uint32_t pedModelType, uint8_t weaponType, uint32_t ammo);

// Teleports the player ped to a position.
bool TeleportPlayer(float x, float y, float z);

// Teleports the entity the player is currently driving.
bool TeleportCurrentVehicle(float x, float y, float z);

// Removes an entity from the world by clearing its pool slot.
// Vehicles use CVehicle + 0x428 status, peds use CPed + 0x530 state.
bool DeleteEntity(uintptr_t entity, bool isVehicle);

// Deletes every ped (and vehicle) that the menu spawned, or that is marked
// as hostile to the player.
int DeleteSpawnedEntities();

}  // namespace sa
