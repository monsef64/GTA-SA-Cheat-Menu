// Addresses.h - GTA: San Andreas v1.0 memory addresses & function addresses.
//
// All addresses come from the GTAMods wiki "Memory Addresses (SA)" and
// "Function Memory Addresses (SA)" pages:
//   https://gtamods.com/wiki/Memory_Addresses_(SA)
//   https://gtamods.com/wiki/Function_Memory_Addresses_(SA)
//
// NOTE: These addresses are confirmed for GTA_SA.EXE v1.0 only. They do NOT
// work on the Steam v2.0/v3.0 builds where the executable was re-laid out.

#pragma once

#include <cstdint>

namespace sa {

// ---------------------------------------------------------------------------
// Base memory addresses (static)
// ---------------------------------------------------------------------------
namespace base {
constexpr uintptr_t kMoney = 0xB7CE50;             // [dword]
constexpr uintptr_t kWantedLevel = 0xBAA420;       // [dword]
constexpr uintptr_t kMaxWantedLevel = 0x8CDEE4;    // [dword]
constexpr uintptr_t kGameSpeed = 0xB7CB64;         // [float] percent
constexpr uintptr_t kGravity = 0x863984;           // [float]
constexpr uintptr_t kInfiniteRun = 0xB7CEE4;       // [byte/bool]
constexpr uintptr_t kPlayerFireproof = 0xB7CEE6;   // [byte/bool]
constexpr uintptr_t kPayNSprayFree = 0x96C009;     // [byte/bool]
constexpr uintptr_t kRadarGreyed = 0xA444A4;       // [byte/bool]
constexpr uintptr_t kPedDensity = 0x8D2530;        // [float] multiplier
constexpr uintptr_t kVehicleDensity = 0x8A5B20;    // [float] multiplier
constexpr uintptr_t kWidescreen = 0xB6F065;        // [byte/bool]

// Cheat switches - one byte each, 1 = cheat enabled.
constexpr uintptr_t kCheatWeaponSet1 = 0x969130;
constexpr uintptr_t kCheatWeaponSet2 = 0x969131;
constexpr uintptr_t kCheatWeaponSet3 = 0x969132;
constexpr uintptr_t kCheatHealthArmorMoney = 0x969133;
constexpr uintptr_t kCheatWantedPlus2 = 0x969134;
constexpr uintptr_t kCheatClearWanted = 0x969135;
constexpr uintptr_t kCheatSunny = 0x969136;
constexpr uintptr_t kCheatVerySunny = 0x969137;
constexpr uintptr_t kCheatOvercast = 0x969138;
constexpr uintptr_t kCheatRainy = 0x969139;
constexpr uintptr_t kCheatFoggy = 0x96913A;
constexpr uintptr_t kCheatFasterClock = 0x96913B;
constexpr uintptr_t kCheatFasterGameplay = 0x96913C;
constexpr uintptr_t kCheatSlowerGameplay = 0x96913D;
constexpr uintptr_t kCheatPedsGolfClub = 0x96913E;
constexpr uintptr_t kCheatBountyOnHead = 0x96913F;
constexpr uintptr_t kCheatEveryoneArmed = 0x969140;
constexpr uintptr_t kCheatSpawnRhino = 0x969141;
constexpr uintptr_t kCheatSpawnBloodring = 0x969142;
constexpr uintptr_t kCheatSpawnRancher = 0x969143;
constexpr uintptr_t kCheatSpawnRacecarA = 0x969144;
constexpr uintptr_t kCheatSpawnRacecarB = 0x969145;
constexpr uintptr_t kCheatSpawnRomero = 0x969146;
constexpr uintptr_t kCheatSpawnStretch = 0x969147;
constexpr uintptr_t kCheatBlowUpCars = 0x96914A;
constexpr uintptr_t kCheatWheelsOnly = 0x96914B;
constexpr uintptr_t kCheatPerfectHandling = 0x96914C;
constexpr uintptr_t kCheatSuicide = 0x96914D;
constexpr uintptr_t kCheatAllGreenLights = 0x96914E;
constexpr uintptr_t kCheatAggressiveDrivers = 0x96914F;
constexpr uintptr_t kCheatPinkTraffic = 0x969150;
constexpr uintptr_t kCheatBlackTraffic = 0x969151;
constexpr uintptr_t kCheatCarsDriveOnWater = 0x969152;
constexpr uintptr_t kCheatBoatsCanFly = 0x969153;
constexpr uintptr_t kCheatCJFat = 0x969154;
constexpr uintptr_t kCheatMaxMuscle = 0x969155;
constexpr uintptr_t kCheatCJSkinny = 0x969156;
constexpr uintptr_t kCheatElvisEverywhere = 0x969157;
constexpr uintptr_t kCheatPedsAttackWithRockets = 0x969158;
constexpr uintptr_t kCheatBeachTheme = 0x969159;
constexpr uintptr_t kCheatGangMembersEverywhere = 0x96915A;
constexpr uintptr_t kCheatGangsControlStreets = 0x96915B;
constexpr uintptr_t kCheatNinjaTheme = 0x96915C;
constexpr uintptr_t kCheatSlutMagnet = 0x96915D;
constexpr uintptr_t kCheatTrafficCheapCars = 0x96915E;
constexpr uintptr_t kCheatTrafficFastCars = 0x96915F;
constexpr uintptr_t kCheatCarsCanFly = 0x969160;
constexpr uintptr_t kCheatHugeBunnyHop = 0x969161;
constexpr uintptr_t kCheatSpawnHydra = 0x969162;
constexpr uintptr_t kCheatSpawnVortex = 0x969163;
constexpr uintptr_t kCheatTankMode = 0x969164;
constexpr uintptr_t kCheatAllCarsHaveNitro = 0x969165;
constexpr uintptr_t kCheatCarsFloatAway = 0x969166;
constexpr uintptr_t kCheatAlwaysMidnight = 0x969167;
constexpr uintptr_t kCheatStopClock = 0x969168;
constexpr uintptr_t kCheatThunderstorm = 0x969169;
constexpr uintptr_t kCheatSandstorm = 0x96916A;
constexpr uintptr_t kCheatMegaJump = 0x96916C;
constexpr uintptr_t kCheatInfiniteHealth = 0x96916D;
constexpr uintptr_t kCheatInfiniteOxygen = 0x96916E;
constexpr uintptr_t kCheatGetParachute = 0x96916F;
constexpr uintptr_t kCheatGetJetpack = 0x969170;
constexpr uintptr_t kCheatLockWanted = 0x969171;
constexpr uintptr_t kCheatSixStarWanted = 0x969172;
constexpr uintptr_t kCheatMegaPunch = 0x969173;
constexpr uintptr_t kCheatNeverHungry = 0x969174;
constexpr uintptr_t kCheatPedsRiot = 0x969175;
constexpr uintptr_t kCheatFunhouseTheme = 0x969176;
constexpr uintptr_t kCheatSlowerGameplay2 = 0x969177;
constexpr uintptr_t kCheatInfiniteAmmo = 0x969178;
constexpr uintptr_t kCheatFullAimWhileDriving = 0x969179;
constexpr uintptr_t kCheatDecreasedTraffic = 0x96917A;
constexpr uintptr_t kCheatCountryTheme = 0x96917B;
constexpr uintptr_t kCheatRecruitAnyone9mm = 0x96917C;
constexpr uintptr_t kCheatRecruitAnyoneAK = 0x96917D;
constexpr uintptr_t kCheatRecruitAnyoneRockets = 0x96917E;
constexpr uintptr_t kCheatMaxRespect = 0x96917F;
constexpr uintptr_t kCheatMaxSexAppeal = 0x969180;
constexpr uintptr_t kCheatMaxStamina = 0x969181;
constexpr uintptr_t kCheatHitmanAllWeapons = 0x969183;
constexpr uintptr_t kCheatSpawnHunter = 0x969184;
constexpr uintptr_t kCheatSpawnQuad = 0x969185;
constexpr uintptr_t kCheatSpawnTanker = 0x969186;
constexpr uintptr_t kCheatSpawnDozer = 0x969187;
constexpr uintptr_t kCheatSpawnStuntPlane = 0x969188;
constexpr uintptr_t kCheatSpawnMonster = 0x969189;
constexpr uintptr_t kCheatAllTaxisHaveNitro = 0x96918B;

// Display settings
constexpr uintptr_t kBrightness = 0xBA6784;   // [dword]
constexpr uintptr_t kLegend = 0xBA6792;       // [byte]
constexpr uintptr_t kRadarMode = 0xBA676C;    // [byte]
constexpr uintptr_t kHudMode = 0xBA6769;      // [byte]
constexpr uintptr_t kSubtitles = 0xBA678C;    // [byte]
constexpr uintptr_t kDrawDistance = 0xBA6788;  // [float]
constexpr uintptr_t kFrameLimiter = 0xBA6794; // [byte]
constexpr uintptr_t kWidescreenOption = 0xBA6793;  // [byte]
constexpr uintptr_t kVisualFxQuality = 0xA9AE54;   // [byte]
constexpr uintptr_t kMipMapping = 0xBA680C;   // [byte]
constexpr uintptr_t kAntialiasing = 0xBA6814; // [byte]
constexpr uintptr_t kResolution = 0xBA6820;   // [byte]

// Sound
constexpr uintptr_t kRadioVolume = 0xBA6798;  // [byte] 0..64
constexpr uintptr_t kSfxVolume = 0xBA6797;     // [byte] 0..64
constexpr uintptr_t kRadioEqualizer = 0xBA6799;
constexpr uintptr_t kRadioAutoTune = 0xBA6795;
constexpr uintptr_t kRadioStationId = 0x8CB7A5;  // [byte]
constexpr uintptr_t kUserTrackPlayMode = 0xBA67F8;

// Controller
constexpr uintptr_t kControllerConfig = 0xBA6818;
constexpr uintptr_t kMouseSensitivity = 0xB6EC1C;  // [float]
constexpr uintptr_t kSteerWithMouse = 0xC1CC02;
constexpr uintptr_t kFlyWithMouse = 0xC1CC03;

// Time
constexpr uintptr_t kTimerGlobal = 0xB610E0;    // [dword] ms, read only
constexpr uintptr_t kTimerGame = 0xB7CB84;      // [dword] ms
constexpr uintptr_t kWeatherTimer = 0xB70158;  // [dword] ms
constexpr uintptr_t kMsPerSecond = 0xB7015C;   // [dword]
constexpr uintptr_t kCurrentWeekday = 0xB7014E; // [byte]
constexpr uintptr_t kDaysPassed = 0xB79038;     // [dword]
constexpr uintptr_t kCurrentHour = 0xB70153;    // [byte]
constexpr uintptr_t kCurrentMinute = 0xB70152;  // [byte]

// Stats / skills
constexpr uintptr_t kStatEnergy = 0xB790B4;
constexpr uintptr_t kStatLungCapacity = 0xB791A4;
constexpr uintptr_t kStatFat = 0xB793D4;      // [float]
constexpr uintptr_t kStatStamina = 0xB793D8;  // [float]
constexpr uintptr_t kStatMuscle = 0xB793DC;   // [float]
constexpr uintptr_t kStatHealth = 0xB793E0;   // [float]
constexpr uintptr_t kStatSexAppeal = 0xB793E4;  // [float]
constexpr uintptr_t kStatRespect = 0xB79480;  // [float]
constexpr uintptr_t kSkillDriving = 0xB790A0;
constexpr uintptr_t kSkillFlying = 0xB7919C;
constexpr uintptr_t kSkillBike = 0xB791B4;
constexpr uintptr_t kSkillCycling = 0xB791B8;
constexpr uintptr_t kSkillGambling = 0xB791C4;

// Weather
constexpr uintptr_t kWeatherLock = 0xC81318;    // [word]
constexpr uintptr_t kUpcomingWeather = 0xC8131C; // [word]
constexpr uintptr_t kCurrentWeather = 0xC81320;  // [word]

// Camera
constexpr uintptr_t kCameraBlock = 0xB6F028;
constexpr uintptr_t kCurrentView = 0xB6F0DC;      // [dword]
constexpr uintptr_t kCarViewDistance = 0xB6F0E0;  // [float]
constexpr uintptr_t kTrueViewDistance = 0xB6F0E8; // [float]

// Misc
constexpr uintptr_t kTextBox = 0xBAA7A0;         // [char(150)]
constexpr uintptr_t kLastTypedChars = 0x969110;   // [char(30)]
constexpr uintptr_t kMenuBase = 0xBA6748;
constexpr uintptr_t kMenuPlayerInMenu = 0xBA6748 + 0x5C;  // [byte]
constexpr uintptr_t kMapTarget = 0xBA6774;        // [byte]
constexpr uintptr_t kRocketsPool = 0xC891A8;
constexpr uintptr_t kBulletsPool = 0xC88740;
}  // namespace base

// ---------------------------------------------------------------------------
// Dynamic addresses (pointers that must be dereferenced)
// ---------------------------------------------------------------------------
namespace dyn {
constexpr uintptr_t kPlayerPed = 0xB6F5F0;      // -> CPed*
constexpr uintptr_t kPlayerData = 0xB7CD98;     // -> CPlayerData
constexpr uintptr_t kCurrentVehicle = 0xBA18FC; // -> CVehicle*
constexpr uintptr_t kVehiclesPool = 0xB74494;  // -> CPoolInfo*
constexpr uintptr_t kPedsPool = 0xB74490;      // -> CPoolInfo*
constexpr uintptr_t kWantedPool = 0xB7CD9C;    // -> CWanted*
constexpr uintptr_t kIgnoredPool = 0xB7CD9C;   // -> CIgnored*
constexpr uintptr_t kVehiclesPoolStart = 0xB6F980;
constexpr uintptr_t kFirstVehicleEntered = 0x969084;
constexpr uintptr_t kCurrentCarId = 0xB700F0;   // [dword] vehicles.ide id
}  // namespace dyn

// ---------------------------------------------------------------------------
// Entity structure offsets
// ---------------------------------------------------------------------------
constexpr uint32_t kPedStructSize = 0x7C4;      // 1988 bytes
constexpr uint32_t kVehicleStructSize = 0xA18;  // 2584 bytes
constexpr uint32_t kWantedStructSize = 668;

// CPed offsets
constexpr uintptr_t kPedPosMatrix = 0x14;      // -> matrix; +0x30 = position
constexpr uintptr_t kEntityModelIndex = 0x22;  // [word] model id (CEntity)
constexpr uintptr_t kPedLocationStatus = 0x2F;  // [byte]
constexpr uintptr_t kPedPhysicalFlags = 0x40;   // 4 bytes of flags
constexpr uintptr_t kPedHealth = 0x540;        // [float]
constexpr uintptr_t kPedMaxHealth = 0x544;     // [float]
constexpr uintptr_t kPedArmor = 0x548;         // [float]
constexpr uintptr_t kPedRotation = 0x558;      // [float] heading
constexpr uintptr_t kPedState = 0x530;         // [dword]
constexpr uintptr_t kPedAnimState = 0x15C;     // [byte]
constexpr uintptr_t kPedPlayerCheck = 0x46C;   // [byte]
constexpr uintptr_t kPedPedProperties = 0x470; // [byte]
constexpr uintptr_t kPedOtherProperties = 0x474;  // [byte]
constexpr uintptr_t kPedWeapons = 0x5A0;        // CWeapon[13]
constexpr uintptr_t kPedCurrentWeaponSlot = 0x718;  // [byte]
constexpr uintptr_t kPedWeaponId = 0x740;           // [dword] default.dat id
constexpr uintptr_t kPedTargetPed = 0x79C;          // -> CPed*
constexpr uintptr_t kPedLastDrivenCar = 0x58C;      // -> CVehicle*
constexpr uintptr_t kPedContactEntity = 0x584;      // -> CEntity*
constexpr uintptr_t kPedPedType = 0x598;            // [dword] ePedType
constexpr uintptr_t kPedDeathTime = 0x4D0;          // [dword]

// CPed special physics flags (byte at kPedPhysicalFlags + 2)
enum PedSpecialFlags : uint8_t {
    kPedFlagSoft = 1 << 0,        // noclip
    kPedFlagFreeze = 1 << 1,
    kPedFlagBulletProof = 1 << 2,
    kPedFlagFireProof = 1 << 3,
    kPedFlagCollisionProof = 1 << 4,
    kPedFlagMeleeProof = 1 << 5,
    kPedFlagExplosionProof = 1 << 7,
};

// CWeapon offsets (28 bytes each)
constexpr uintptr_t kWeaponType = 0x0;    // [dword]
constexpr uintptr_t kWeaponState = 0x4;   // [dword]
constexpr uintptr_t kWeaponAmmoInClip = 0x8;
constexpr uintptr_t kWeaponTotalAmmo = 0xC;

// CVehicle offsets
constexpr uintptr_t kVehPosMatrix = 0x14;      // -> matrix; +0x30 = position
constexpr uintptr_t kVehModelId = 0x22;        // [word] vehicles.ide id
constexpr uintptr_t kVehEntityStatus = 0x36;   // [byte]
constexpr uintptr_t kVehPhysicalFlags = 0x40;
constexpr uintptr_t kVehMass = 0x8C;           // [float]
constexpr uintptr_t kVehSirenOn = 0x42D;       // [byte]
constexpr uintptr_t kVehBodyColor = 0x434;     // [byte]
constexpr uintptr_t kVehStripeColor = 0x435;   // [byte]
constexpr uintptr_t kVehDriver = 0x460;        // -> CPed*
constexpr uintptr_t kVehPassenger1 = 0x464;
constexpr uintptr_t kVehNitroCount = 0x8A4;    // [float]
constexpr uintptr_t kVehNitroAvailable = 0x48A; // [byte]
constexpr uintptr_t kVehHealth = 0x4C0;        // [float]
constexpr uintptr_t kVehDoorLocked = 0x4F8;    // [dword]
constexpr uintptr_t kVehBurnTimer = 0x8E4;     // [float]
constexpr uintptr_t kVehCarType = 0x590;       // [byte]
constexpr uintptr_t kVehEngineState = 0x428;   // [byte]

// eEntityStatus
enum EntityStatus : uint8_t {
    kStatusPlayer = 0x02,
    kStatusPlayerPlayback = 0x0A,
    kStatusSimple = 0x12,
    kStatusPhysics = 0x1A,
    kStatusAbandoned = 0x22,
    kStatusWrecked = 0x2A,
    kStatusTrainMoving = 0x32,
    kStatusTrainNotMoving = 0x3A,
    kStatusRemoteControlled = 0x42,
    kStatusPlayerDisabled = 0x4A,
    kStatusTrailer = 0x52,
    kStatusSimpleTrailer = 0x5A,
    kStatusGhost = 0x62,
};

// ePedType - the broad category a ped belongs to. It is passed to the
// CPed constructor and decides which default behaviour the ped gets.
enum PedType : uint32_t {
    kPedTypePlayer = 0,
    kPedTypeCivilian = 4,
    kPedTypeCop = 6,
    kPedTypeGang = 7,
    kPedTypeSwat = 8,
    kPedTypeFBI = 9,
    kPedTypeArmy = 10,
    kPedTypeEmergency = 12,
};

// eWeaponType (weapon slot index is the same value here)
enum WeaponType : uint8_t {
    kWeaponFist = 0,
    kWeaponBrassKnuckles = 1,
    kWeaponGolfClub = 2,
    kWeaponNitestick = 3,
    kWeaponKnife = 4,
    kWeaponBaseballBat = 5,
    kWeaponShovel = 6,
    kWeaponPoolCue = 7,
    kWeaponKatana = 8,
    kWeaponChainsaw = 9,
    kWeaponCane = 15,
    kWeaponGrenade = 16,
    kWeaponMolotov = 18,
    kWeaponPistol = 22,
    kWeaponSilencedPistol = 23,
    kWeaponDesertEagle = 24,
    kWeaponShotgun = 25,
    kWeaponSawnOffShotgun = 26,
    kWeaponSPAS12 = 27,
    kWeaponMicroUzi = 28,
    kWeaponMP5 = 29,
    kWeaponAK47 = 30,
    kWeaponM4 = 31,
    kWeaponTEC9 = 32,
    kWeaponCountryRifle = 33,
    kWeaponSniperRifle = 34,
    kWeaponRocketLauncher = 35,
    kWeaponHeatSeekingRPG = 36,
    kWeaponFlamethrower = 37,
    kWeaponMinigun = 38,
    kWeaponRemoteExplosives = 39,
    kWeaponDetonator = 40,
    kWeaponFireExtinguisher = 42,
    kWeaponCamera = 43,
    kWeaponNVGoggles = 44,
    kWeaponIRGoggles = 45,
    kWeaponParachute = 46,
    kWeaponFlowers = 14,
    kWeaponFlare = 58,
};

// Number of valid weapon slots in the CWeapon array on CPed.
constexpr int kNumWeaponSlots = 13;

// ---------------------------------------------------------------------------
// Function addresses (v1.0)
// ---------------------------------------------------------------------------
namespace funcs {
constexpr uintptr_t kAllocatePools = 0x550F10;
constexpr uintptr_t kCreateCar = 0x43A0B6;              // CVehicle::CreateCar
constexpr uintptr_t kCreateTrain = 0x6F7550;
constexpr uintptr_t kDestroyTrain = 0x6F5DC0;
constexpr uintptr_t kGetPlayerPed = 0x56E210;          // CPed::GetPlayerPed
constexpr uintptr_t kCheatCreateCar = 0x407851;         // CCheat::CreateCar
constexpr uintptr_t kCheatCreateCar408 = 0x43A510;    // CCheat::CreateCar408 (Comet)
constexpr uintptr_t kCheatCreateCar409 = 0x43A500;    // CCheat::CreateCar409 (Coquette)
constexpr uintptr_t kCheatCreateCar432 = 0x43A4A0;    // CCheat::CreateCar432 (Merit)
constexpr uintptr_t kCheatCreateCar442 = 0x43A4F0;    // CCheat::CreateCar442 (Phantom)
constexpr uintptr_t kCheatCreateCar457 = 0x43A520;    // CCheat::CreateCar457 (Tuner)
constexpr uintptr_t kCheatCreateCar502 = 0x43A4D0;    // CCheat::CreateCar502
constexpr uintptr_t kCheatCreateCar503 = 0x43A4E0;    // CCheat::CreateCar503
constexpr uintptr_t kCheatCreateCar504 = 0x43A4B0;    // CCheat::CreateCar504
constexpr uintptr_t kCheatCreateCar505 = 0x43A4C0;    // CCheat::CreateCar505
constexpr uintptr_t kCheatCreateCarDozer = 0x43A660;  // CCheat::CreateCarDozer
constexpr uintptr_t kCheatCreateCarHunter = 0x43A550; // CCheat::CreateCarHunter
constexpr uintptr_t kCheatCreateCarQuad = 0x43A560;   // CCheat::CreateCarQuad
constexpr uintptr_t kCheatCreateCarMonster = 0x43A680; // CCheat::CreateCarMonster
constexpr uintptr_t kCheatCreateCarStuntPlane = 0x43A670; // CCheat::CreateCarStuntPlane
constexpr uintptr_t kCheatCreateCarTanker = 0x43A570; // CCheat::CreateCarTanker
constexpr uintptr_t kCheatProcess = 0x438480;
constexpr uintptr_t kCheatWeaponSet1 = 0x4385B0;
constexpr uintptr_t kCheatWeaponSet2 = 0x438890;
constexpr uintptr_t kCheatWeaponSet3 = 0x438B30;
constexpr uintptr_t kCheatEveryoneArmed = 0x439C70;
constexpr uintptr_t kCheatSetSkillsMax = 0x4399D0;
constexpr uintptr_t kCheatAddArmourMoneyHealth = 0x438E40;
constexpr uintptr_t kCheatClearWantedLevel = 0x438F20;
constexpr uintptr_t kCheatSetWantedLevel = 0x609F50;
constexpr uintptr_t kCheatIncreaseGameSpeed = 0x438F90;
constexpr uintptr_t kCheatDecreaseGameSpeed = 0x438FC0;
constexpr uintptr_t kCheatJetpack = 0x439900;
constexpr uintptr_t kCheatPedsAttackAll = 0x439B20;
constexpr uintptr_t kCheatSetWeather0 = 0x438F50;
constexpr uintptr_t kCheatSetWeather1 = 0x438F40;
constexpr uintptr_t kCheatSetWeather4 = 0x438F70;
constexpr uintptr_t kCheatSetWeather9 = 0x438F80;
constexpr uintptr_t kCreatePoliceInCity = 0x407C50;

// Ped creation. The game has no "CreatePed" helper, but the CPed constructor
// and the model setter are ordinary member functions that can be called
// directly once a pool slot has been claimed.
//
//   CPed::CPed(ePedType)          - 0x5E8030 (thiscall)
//   CEntity::SetModelIndex(uint)  - 0x5E4880 (thiscall)
//   CEntity::Teleport(vec, bool)  - virtual, vtable slot 14
//
// The pool allocation helpers live in the CPool template:
//   CPool::IsFreeSlotAtIndex(int) - 0x404940
//   CPool::SetFreeAt(int, bool)   - 0x404970
//   CPool::CreateAtRef(int)       - 0x59F610
constexpr uintptr_t kPedConstructor = 0x5E8030;
constexpr uintptr_t kEntitySetModelIndex = 0x5E4880;
constexpr uintptr_t kPoolIsFreeSlotAtIndex = 0x404940;
constexpr uintptr_t kPoolSetFreeAt = 0x404970;
constexpr uintptr_t kPoolCreateAtRef = 0x59F610;

constexpr uintptr_t kFade = 0x50AC20;
constexpr uintptr_t kScriptCreateNewThread = 0x156A2A0;
constexpr uintptr_t kCWeaponInitialise = 0x73A300;
constexpr uintptr_t kExplosionInitialise = 0x736A40;
constexpr uintptr_t kDirect3DCreate9 = 0x807C2B;
}  // namespace funcs

}  // namespace sa
