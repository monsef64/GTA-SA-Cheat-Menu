#include "Entities.h"

#include <cmath>
#include <cstring>

#include "Addresses.h"
#include "PedModels.h"
#include "VehicleModels.h"
#include "Weapons.h"

namespace sa {
namespace {

// The game stores peds / vehicles in pools. Each pool is described by a
// "pool info" structure that we can read from the well known addresses.
bool ReadPoolInfo(uintptr_t poolInfoAddress, PoolInfo* out) {
    if (poolInfoAddress == 0 || out == nullptr) return false;
    out->entities = ReadPointer(poolInfoAddress + 0x0);
    out->usageMap = ReadPointer(poolInfoAddress + 0x4);
    out->maxCount = ReadU32(poolInfoAddress + 0x8);
    out->currentCount = ReadU32(poolInfoAddress + 0xC);
    return out->entities != 0 && out->maxCount > 0 && out->maxCount < 4096;
}

// Walks a pool and returns every entity whose usage byte marks a live slot.
std::vector<Entity> EnumeratePool(uintptr_t poolInfoAddress, uint32_t structSize,
                                   bool skipFirstSlot) {
    std::vector<Entity> result;
    PoolInfo pool;
    if (!ReadPoolInfo(poolInfoAddress, &pool)) return result;

    result.reserve(pool.currentCount);
    // Slot 0 of the ped pool is always the player ped, so it is skipped by
    // starting the loop at 1 instead of filtering the result afterwards.
    const uint32_t firstSlot = skipFirstSlot ? 1 : 0;

    for (uint32_t i = firstSlot; i < pool.maxCount; ++i) {
        const uint8_t usageByte = ReadU8(pool.usageMap + i);
        if (!PoolSlotInUse(usageByte)) continue;

        const uintptr_t entity = pool.entities + static_cast<uintptr_t>(i) * structSize;
        if (entity == 0) continue;

        Entity e;
        e.address = entity;
        e.handle = i;
        if (!ReadEntityPosition(entity, &e.position)) continue;
        e.heading = HeadingToDegrees(ReadFloat(entity + kPedRotation));
        result.push_back(e);
    }

    return result;
}

struct ModelNameEntry {
    uint32_t id;
    const char* name;
};

// Model ids of the peds the ESP treats as enemies. These are the
// CEntity::m_wModelIndex values from the stock peds.ide.
constexpr uint16_t kCopModels[] = {
    3,    // cdeput
    4,    // sfpdm1
    66,   // bmypol1
    67,   // bmypol2
    119,  // lvpdm1
    190,  // copgrl3
    274,  // laemt1
    275,  // lvemt1
    276,  // sfemt1
    277,  // lafd1
    278,  // lvfd1
    279,  // sffd1
    280,  // lapd1
    281,  // sfpd1
    282,  // lvpd1
    283,  // csher
    284,  // lapdm1
    285,  // swat
    286,  // fbi
    287,  // army
    288,  // dsher
};

// Model ids of the gang members.
constexpr uint16_t kGangModels[] = {
    35,   // hmogar
    36,   // wmygol1
    37,   // wmygol2
    102,  // ballas1
    103,  // ballas2
    104,  // ballas3
    105,  // fam1
    106,  // fam2
    107,  // fam3
    108,  // lsv1
    109,  // lsv2
    110,  // lsv3
    111,  // maffa
    112,  // maffb
    113,  // mafboss
    114,  // vla1
    115,  // vla2
    116,  // vla3
    117,  // triada
    118,  // triadb
    120,  // triboss
    121,  // dnb1
    122,  // dnb2
    123,  // dnb3
    124,  // vmaff1
    125,  // vmaff2
    126,  // vmaff3
    127,  // vmaff4
    128,  // dnmylc
    129,  // dnfolc1
    130,  // dnfolc2
    131,  // dnfylc
    132,  // dnmolc1
    133,  // dnmolc2
    144,  // bmycg
    145,  // wfycrk
    146,  // hmycm
    151,  // dwfylc1
    152,  // wfypro
    163,  // bmyboun
    164,  // wmyboun
    165,  // wmomib
    166,  // bmymib
    167,  // wmybell
    173,  // sfr1
    174,  // sfr2
    175,  // sfr3
    177,  // wmybar
    180,  // bmytatt
    185,  // sbmyri
    186,  // somyri
    187,  // somybu
    191,  // gungrl3
    192,  // mecgrl3
    193,  // nurgrl3
    194,  // crogrl3
    195,  // gangrl3
    201,  // dwfylc2
    202,  // dwmylc2
    203,  // omykara
    204,  // wmykara
    205,  // wfyburg
    206,  // vwmycd
    209,  // omonood
    210,  // omoboat
    211,  // wfyclot
    214,  // vwfywai
    217,  // wmyclot
    220,  // sbmocd
    221,  // sbmori
    222,  // sbmost
    223,  // shmycr
    227,  // somobu
    228,  // somori
    229,  // somost
    234,  // swmocd
    235,  // swmori
    236,  // swmost
    240,  // swmyri
    243,  // sfypro
    249,  // bmypimp
    250,  // swmycr
    251,  // wfylg
    253,  // bmosec
    255,  // wmych
    256,  // sbfystr
    257,  // swfystr
    258,  // heck1
    259,  // heck2
    260,  // bmycon
    261,  // wmycd1
    262,  // bmocd
    263,  // vwfywa2
    264,  // wmoice
};

bool ModelIdInList(uint16_t modelId, const uint16_t* list, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        if (list[i] == modelId) return true;
    }
    return false;
}

}  // namespace

bool GetPedPool(PoolInfo* out) { return ReadPoolInfo(dyn::kPedsPool, out); }
bool GetVehiclePool(PoolInfo* out) { return ReadPoolInfo(dyn::kVehiclesPool, out); }

bool ReadPoolInfoAddress(uintptr_t poolInfoAddress, PoolInfo* out) {
    return ReadPoolInfo(poolInfoAddress, out);
}

bool FindPoolSlot(uintptr_t poolInfoAddress, uint32_t structSize, uintptr_t entity,
                  uint32_t* slotOut) {
    if (entity == 0 || slotOut == nullptr) return false;
    PoolInfo pool;
    if (!ReadPoolInfo(poolInfoAddress, &pool)) return false;
    if (entity < pool.entities) return false;

    const uintptr_t offset = entity - pool.entities;
    if (offset % structSize != 0) return false;

    const uint32_t slot = static_cast<uint32_t>(offset / structSize);
    if (slot >= pool.maxCount) return false;

    *slotOut = slot;
    return true;
}

std::vector<Entity> GetPeds(bool includePlayer) {
    return EnumeratePool(dyn::kPedsPool, kPedStructSize, !includePlayer);
}

std::vector<Entity> GetVehicles() {
    return EnumeratePool(dyn::kVehiclesPool, kVehicleStructSize, false);
}

uintptr_t GetPlayerPed() {
    const uintptr_t ped = ReadPointer(dyn::kPlayerPed);
    if (ped == 0) {
        // Fall back to the player data block which holds the ped pointer.
        const uintptr_t data = ReadPointer(dyn::kPlayerData);
        if (data != 0) return ReadPointer(data + 0x0);
    }
    return ped;
}

uintptr_t GetPlayerVehicle() {
    const uintptr_t vehicle = ReadPointer(dyn::kCurrentVehicle);
    // On foot the game stores the ped pointer here, so sanity check that the
    // pointer really looks like a vehicle before returning it.
    if (vehicle == 0) return 0;
    const uint8_t status = ReadU8(vehicle + kVehEntityStatus);
    if (status != kStatusPlayer && status != kStatusPlayerDisabled &&
        status != kStatusRemoteControlled && status != kStatusPlayerPlayback) {
        return 0;
    }
    return vehicle;
}

bool IsCopPed(uintptr_t ped) {
    if (ped == 0) return false;
    const uint16_t model = GetPedModelIndex(ped);
    // Police, SWAT, FBI and Army peds.
    return ModelIdInList(model, kCopModels,
                          sizeof(kCopModels) / sizeof(kCopModels[0]));
}

bool IsGangPed(uintptr_t ped) {
    if (ped == 0) return false;
    const uint16_t model = GetPedModelIndex(ped);
    return ModelIdInList(model, kGangModels,
                          sizeof(kGangModels) / sizeof(kGangModels[0]));
}

bool IsPedDead(uintptr_t ped) {
    if (ped == 0) return false;
    const float health = ReadFloat(ped + kPedHealth);
    return health <= 0.0f;
}

uint16_t GetPedModelIndex(uintptr_t ped) {
    if (ped == 0) return 0;
    return ReadU16(ped + kEntityModelIndex);
}

uint32_t GetPedType(uintptr_t ped) {
    if (ped == 0) return 0;
    return ReadU32(ped + kPedPedType);
}

uint16_t GetVehicleModelId(uintptr_t vehicle) {
    if (vehicle == 0) return 0;
    return ReadU16(vehicle + kVehModelId);
}

std::string PedModelName(uint32_t modelType) {
    return sa::PedModelName(static_cast<uint16_t>(modelType));
}

bool TeleportEntity(uintptr_t entity, const Vec3& pos, float headingDegrees) {
    if (entity == 0) return false;
    return WriteEntityTransform(entity, pos, DegreesToHeading(headingDegrees));
}

Vec3 PositionRelativeToPlayer(float forward, float right) {
    Vec3 result{};
    const uintptr_t ped = GetPlayerPed();
    if (ped == 0) return result;
    if (!ReadEntityPosition(ped, &result)) return result;

    const float heading = ReadFloat(ped + kPedRotation);  // radians
    const float cosH = std::cos(heading);
    const float sinH = std::sin(heading);

    // Forward vector is (-sin, cos), right vector is (cos, sin).
    result.x += forward * -sinH + right * cosH;
    result.y += forward * cosH + right * sinH;
    return result;
}

bool GiveWeapon(uintptr_t ped, uint8_t weaponType, uint32_t ammo) {
    if (ped == 0) return false;
    const int slot = WeaponSlotForType(weaponType);
    if (slot < 0) return false;

    const uintptr_t weapon = ped + kPedWeapons + static_cast<uintptr_t>(slot) * 0x1C;

    // The CWeapon struct is:
    //   +0  dword  type
    //   +4  dword  state (0 = ready)
    //   +8  dword  ammo in clip
    //   +12 dword  total ammo
    //   +16 float  time for next shot
    //   +20 .. +27 unknown
    WriteU32(weapon + kWeaponType, weaponType);
    WriteU32(weapon + kWeaponState, 0);  // WEAPONSTATE_READY
    WriteU32(weapon + kWeaponAmmoInClip, ammo);
    WriteU32(weapon + kWeaponTotalAmmo, ammo);
    WriteFloat(weapon + 0x10, 0.0f);  // time for next shot

    // Clear the remaining bytes so no stale state is left over.
    WriteU32(weapon + 0x14, 0);
    WriteU32(weapon + 0x18, 0);

    // Select the slot we just filled so the weapon is drawn immediately.
    WriteU8(ped + kPedCurrentWeaponSlot, static_cast<uint8_t>(slot));
    return true;
}

}  // namespace sa
