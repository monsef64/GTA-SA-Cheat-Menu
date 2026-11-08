#include "Spawner.h"

#include <cmath>
#include <vector>

#include "Addresses.h"
#include "Entities.h"
#include "Memory.h"
#include "Platform.h"
#include "Weapons.h"

namespace sa {
namespace {

// CVehicle::CreateCar (0x43A0B6) takes a single argument - the vehicles.ide
// model id - and does everything else itself: it loads the model, works out
// the coordinates in front of the player and creates the vehicle.
//
// See "Spawn a vehicle" on the GTAMods wiki:
//     0AA5: call 0x0043A0B6 num_params 1 pop 1 #INFERNUS
//
// The function returns a pointer to the newly created CVehicle.
typedef uintptr_t(SA_CDECL* CreateCarFn)(int modelId);

// CCheat::CreateCar (0x407851) is the wrapper used by the cheat codes. It
// takes the same single model id argument and additionally handles the
// special cheat-only vehicles (Rhino, Hunter, Hydra, ...).
typedef uintptr_t(SA_CDECL* CheatCreateCarFn)(int modelId);

// CPed::CPed(ePedType) is the game's own ped constructor. Calling it gives
// the ped a valid vtable, an intelligence block, a task manager and all the
// other members the game expects - writing the struct by hand does not.
typedef void(SA_THISCALLFN* PedConstructorFn)(void* ped, uint32_t pedType);

// CEntity::SetModelIndex(unsigned int) swaps the ped's model.
typedef void(SA_THISCALLFN* SetModelIndexFn)(void* entity, uint32_t modelIndex);

// CPool::SetFreeAt(int slot, bool free) marks a pool slot as used / free.
typedef void(SA_THISCALLFN* SetFreeAtFn)(void* pool, int slot, bool free);

CreateCarFn GetCreateCar() {
    return reinterpret_cast<CreateCarFn>(ResolveAddress(funcs::kCreateCar));
}

CheatCreateCarFn GetCheatCreateCar() {
    return reinterpret_cast<CheatCreateCarFn>(ResolveAddress(funcs::kCheatCreateCar));
}

PedConstructorFn GetPedConstructor() {
    return reinterpret_cast<PedConstructorFn>(ResolveAddress(funcs::kPedConstructor));
}

SetModelIndexFn GetSetModelIndex() {
    return reinterpret_cast<SetModelIndexFn>(ResolveAddress(funcs::kEntitySetModelIndex));
}

SetFreeAtFn GetSetFreeAt() {
    return reinterpret_cast<SetFreeAtFn>(ResolveAddress(funcs::kPoolSetFreeAt));
}

// Turns a SpawnOffset into a world position relative to the player.
Vec3 ResolvePosition(SpawnOffset offset, float distance) {
    switch (offset) {
        case SpawnOffset::AtPlayer: return PositionRelativeToPlayer(0.0f, 0.0f);
        case SpawnOffset::InFront: return PositionRelativeToPlayer(distance, 0.0f);
        case SpawnOffset::ToTheRight: return PositionRelativeToPlayer(0.0f, distance);
        case SpawnOffset::SlightlyAbove: {
            Vec3 pos = PositionRelativeToPlayer(distance, 0.0f);
            pos.z += 5.0f;
            return pos;
        }
    }
    return PositionRelativeToPlayer(distance, 0.0f);
}

float PlayerHeadingDegrees() {
    const uintptr_t ped = GetPlayerPed();
    if (ped == 0) return 0.0f;
    return HeadingToDegrees(ReadFloat(ped + kPedRotation));
}

// Tracks what we spawned so it can be cleaned up later.
struct SpawnTracker {
    std::vector<uintptr_t> vehicles;
    std::vector<uintptr_t> peds;
};

SpawnTracker& Tracker() {
    static SpawnTracker tracker;
    return tracker;
}

// The game spawns the vehicle in front of the player itself. To find the
// result we diff the vehicle pool against a snapshot taken before the call:
// the vehicle that appeared is the one we asked for.
uintptr_t FindNewVehicle(const std::vector<uintptr_t>& before) {
    const std::vector<Entity> vehicles = GetVehicles();
    for (const Entity& vehicle : vehicles) {
        bool existed = false;
        for (uintptr_t known : before) {
            if (known == vehicle.address) {
                existed = true;
                break;
            }
        }
        if (!existed) return vehicle.address;
    }
    return 0;
}

// Snapshot of the vehicle pool, used to detect newly created vehicles.
std::vector<uintptr_t> SnapshotVehicles() {
    std::vector<uintptr_t> result;
    const std::vector<Entity> vehicles = GetVehicles();
    result.reserve(vehicles.size());
    for (const Entity& vehicle : vehicles) result.push_back(vehicle.address);
    return result;
}

}  // namespace

uintptr_t SpawnVehicle(uint16_t modelId, SpawnOffset offset) {
    if (!IsAttached()) return 0;

    // CreateCar always places the vehicle in front of the player, so the
    // requested offset is applied afterwards by moving the vehicle.
    const Vec3 target = ResolvePosition(offset, 5.0f);

    // Snapshot the pool first so the spawned vehicle can be identified even
    // when the game function does not hand the pointer back.
    const std::vector<uintptr_t> before = SnapshotVehicles();

    uintptr_t vehicle = 0;
    try {
        vehicle = GetCreateCar()(static_cast<int>(modelId));
    } catch (...) {
        vehicle = 0;
    }

    // The cheat wrapper knows about the special vehicles, so use it as a
    // fallback when the plain function did not produce anything.
    if (vehicle == 0) {
        try {
            vehicle = GetCheatCreateCar()(static_cast<int>(modelId));
        } catch (...) {
            vehicle = 0;
        }
    }

    if (vehicle == 0) {
        // The function may not return the pointer on some builds. Look for a
        // vehicle that appeared in the pool instead.
        vehicle = FindNewVehicle(before);
    }

    if (vehicle == 0) return 0;

    // Move the vehicle to the requested offset and face it the same way as
    // the player.
    TeleportEntity(vehicle, target, PlayerHeadingDegrees());

    Tracker().vehicles.push_back(vehicle);
    return vehicle;
}

uintptr_t SpawnPed(uint32_t pedModelType, SpawnOffset offset) {
    if (!IsAttached()) return 0;

    const Vec3 pos = ResolvePosition(offset, 2.0f);
    const float heading = PlayerHeadingDegrees();

    // GTA:SA has no "CreatePed" helper, so the ped is built the same way the
    // game builds one:
    //   1. claim a free slot in the ped pool,
    //   2. run the real CPed constructor on it,
    //   3. call SetModelIndex() to pick the model.
    //
    // Writing the struct by hand does not work: the game expects a valid
    // vtable, a CPedIntelligence block and a task manager, all of which the
    // constructor sets up.
    PoolInfo pool;
    if (!GetPedPool(&pool)) return 0;

    // Find a free slot in the ped pool. Slot 0 is always the player.
    uint32_t slot = 0xFFFFFFFF;
    for (uint32_t i = 1; i < pool.maxCount; ++i) {
        if (PoolSlotInUse(ReadU8(pool.usageMap + i))) continue;
        slot = i;
        break;
    }
    if (slot == 0xFFFFFFFF) return 0;

    const uintptr_t ped = pool.entities + static_cast<uintptr_t>(slot) * kPedStructSize;
    if (ped == 0) return 0;

    // Mark the slot as used so the game does not hand it out again. The byte
    // map stores the SCM handle id in bits 0-6 and the "empty" flag in bit 7,
    // so a used slot has bit 7 clear and the id incremented.
    const uint8_t previous = ReadU8(pool.usageMap + slot);
    WriteU8(pool.usageMap + slot, static_cast<uint8_t>((previous + 1) & 0x7F));

    // Build the ped with the game's own constructor. ePedType 4 is a plain
    // civilian, which is the right base for every model we spawn.
    try {
        GetPedConstructor()(reinterpret_cast<void*>(ped), kPedTypeCivilian);
    } catch (...) {
        // A failure here leaves the slot half-initialised; give it back.
        WriteU8(pool.usageMap + slot, static_cast<uint8_t>(previous & 0x80));
        return 0;
    }

    // Swap in the requested model.
    try {
        GetSetModelIndex()(reinterpret_cast<void*>(ped), pedModelType);
    } catch (...) {
        WriteU8(pool.usageMap + slot, static_cast<uint8_t>(previous & 0x80));
        return 0;
    }

    // The constructor resets health and armour, so set them afterwards.
    WriteFloat(ped + kPedHealth, 100.0f);
    WriteFloat(ped + kPedMaxHealth, 100.0f);
    WriteFloat(ped + kPedArmor, 0.0f);

    TeleportEntity(ped, pos, heading);

    Tracker().peds.push_back(ped);
    return ped;
}

uintptr_t SpawnArmedPed(uint32_t pedModelType, uint8_t weaponType, uint32_t ammo) {
    const uintptr_t ped = SpawnPed(pedModelType, SpawnOffset::ToTheRight);
    if (ped == 0) return 0;
    GiveWeapon(ped, weaponType, ammo);
    return ped;
}

bool TeleportPlayer(float x, float y, float z) {
    const uintptr_t ped = GetPlayerPed();
    if (ped == 0) return false;
    Vec3 pos{x, y, z};
    return WriteEntityTransform(ped, pos, ReadFloat(ped + kPedRotation));
}

bool TeleportCurrentVehicle(float x, float y, float z) {
    const uintptr_t vehicle = GetPlayerVehicle();
    if (vehicle == 0) return false;
    Vec3 pos{x, y, z};
    return WriteEntityTransform(vehicle, pos, ReadFloat(vehicle + kPedRotation));
}

bool DeleteEntity(uintptr_t entity, bool isVehicle) {
    if (entity == 0) return false;

    const uintptr_t poolInfo = isVehicle ? dyn::kVehiclesPool : dyn::kPedsPool;
    const uint32_t structSize = isVehicle ? kVehicleStructSize : kPedStructSize;

    PoolInfo pool;
    uint32_t slot = 0;
    if (!ReadPoolInfoAddress(poolInfo, &pool)) return false;
    if (!FindPoolSlot(poolInfo, structSize, entity, &slot)) return false;

    // Mark the entity as wasted / wrecked first so the game stops using it,
    // then release the pool slot.
    if (isVehicle) {
        WriteU8(entity + kVehEntityStatus, kStatusWrecked);
    } else {
        WriteU32(entity + kPedState, 55);  // wasted
        WriteFloat(entity + kPedHealth, 0.0f);
    }

    // Freeing the slot means setting the "empty" bit (bit 7) in the byte
    // map. The SCM handle id in bits 0-6 is left untouched so the handle is
    // not reused while the game still references it.
    const uint8_t usage = ReadU8(pool.usageMap + slot);
    WriteU8(pool.usageMap + slot, static_cast<uint8_t>(usage | kPoolSlotEmptyFlag));
    return true;
}

int DeleteSpawnedEntities() {
    SpawnTracker& tracker = Tracker();
    int deleted = 0;

    for (uintptr_t vehicle : tracker.vehicles) {
        if (DeleteEntity(vehicle, true)) ++deleted;
    }
    for (uintptr_t ped : tracker.peds) {
        if (DeleteEntity(ped, false)) ++deleted;
    }

    tracker.vehicles.clear();
    tracker.peds.clear();
    return deleted;
}

}  // namespace sa
