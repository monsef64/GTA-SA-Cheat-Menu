// Entities.h - Helpers that walk the CPed / CVehicle pools and expose the
// game entities to the rest of the menu.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Memory.h"

namespace sa {

// A single entity (ped or vehicle) found in a pool.
struct Entity {
    uintptr_t address = 0;  // address of the CPed / CVehicle struct
    uintptr_t handle = 0;   // pool slot index (handle)
    Vec3 position{};
    float heading = 0.0f;   // degrees
};

// Pool info layout shared by the ped and vehicle pools:
//   +0 = pointer to the first entity
//   +4 = pointer to a byte map marking which slots are in use
//   +8 = max number of entities
//   +12 = current number of entities
struct PoolInfo {
    uintptr_t entities = 0;
    uintptr_t usageMap = 0;
    uint32_t maxCount = 0;
    uint32_t currentCount = 0;
};

// The game's pool "byte map" is a bitfield per slot:
//
//   bits 0-6 : the slot's SCM handle id (incremented every time the slot is
//               reused)
//   bit  7   : "bEmpty" - the slot is free when this bit is SET
//
// So a slot is *in use* when bit 7 is clear. Getting this backwards makes the
// ESP enumerate the free slots instead of the live entities.
constexpr uint8_t kPoolSlotEmptyFlag = 0x80;

// True when the pool slot described by `usageByte` holds a live entity.
inline bool PoolSlotInUse(uint8_t usageByte) {
    return (usageByte & kPoolSlotEmptyFlag) == 0;
}

bool GetPedPool(PoolInfo* out);
bool GetVehiclePool(PoolInfo* out);

// Reads a pool info structure from an arbitrary pool address.
bool ReadPoolInfoAddress(uintptr_t poolInfoAddress, PoolInfo* out);

// Given an entity address, works out which pool slot it occupies.
// Returns false when the entity does not belong to the pool.
bool FindPoolSlot(uintptr_t poolInfoAddress, uint32_t structSize, uintptr_t entity,
                  uint32_t* slotOut);

// Enumerates all live peds (excluding the player ped).
std::vector<Entity> GetPeds(bool includePlayer = false);
// Enumerates all live vehicles.
std::vector<Entity> GetVehicles();

// The player's ped, or 0 if unavailable.
uintptr_t GetPlayerPed();
// The vehicle the player is currently driving, or 0 when on foot.
uintptr_t GetPlayerVehicle();

// True when the ped is one of the law-enforcement models (cop / SWAT / FBI /
// army). Used by the ESP to colour enemies differently from civilians.
bool IsCopPed(uintptr_t ped);
// True when the ped belongs to a gang (Ballas / Vagos / Grove Street / ...).
bool IsGangPed(uintptr_t ped);
// True when the ped is dead / wasted.
bool IsPedDead(uintptr_t ped);

// The model index stored in the ped (CEntity::m_wModelIndex, peds.ide id).
uint16_t GetPedModelIndex(uintptr_t ped);
// The ePedType category of the ped (player / civilian / cop / gang / ...).
uint32_t GetPedType(uintptr_t ped);
// Vehicle model id from vehicles.ide.
uint16_t GetVehicleModelId(uintptr_t vehicle);

// Human readable names ---------------------------------------------------
std::string PedModelName(uint32_t modelType);
// VehicleModelName() lives in VehicleModels.h.
// PedModelName(uint16_t) lives in PedModels.h.

// Teleports an entity to a world position.
bool TeleportEntity(uintptr_t entity, const Vec3& pos, float headingDegrees);

// Places an entity relative to the player: `forward` meters ahead of the
// player, `right` meters to the player's right.
Vec3 PositionRelativeToPlayer(float forward, float right);

// Gives a ped a weapon directly by writing the CWeapon struct.
bool GiveWeapon(uintptr_t ped, uint8_t weaponType, uint32_t ammo);

}  // namespace sa
