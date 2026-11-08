// Memory.h - Thin, safe-ish wrappers around raw memory access in the game
// process. All reads/writes are done through this so the rest of the code
// stays readable.

#pragma once

#include <cstdint>
#include <string>

namespace sa {

// The base address of GTA_SA.EXE once it is loaded.
uintptr_t GameBase();

// Records the base address of the game module. Called once at start-up.
void SetGameBase(uintptr_t base);

// True once we have a valid module base (i.e. injected into the game).
bool IsAttached();

// Turns an absolute wiki address (0x004xxxxx / 0x00Bxxxxx) into a usable
// pointer inside the running game. GTA:SA v1.0 is linked at a fixed base of
// 0x400000, so this normally returns the address unchanged.
uintptr_t ResolveAddress(uintptr_t absoluteAddress);

// Raw access -------------------------------------------------------------
bool ReadBytes(uintptr_t address, void* out, size_t size);
bool WriteBytes(uintptr_t address, const void* in, size_t size);

template <typename T>
inline bool Read(uintptr_t address, T* out) {
    return ReadBytes(address, out, sizeof(T));
}

template <typename T>
inline bool Write(uintptr_t address, const T& value) {
    return WriteBytes(address, &value, sizeof(T));
}

template <typename T>
inline T ReadOr(uintptr_t address, T fallback = T{}) {
    T value{};
    if (!Read<T>(address, &value)) return fallback;
    return value;
}

template <typename T>
inline bool WriteIfValid(uintptr_t address, T value) {
    if (address == 0) return false;
    return Write<T>(address, value);
}

// Pointer helpers ---------------------------------------------------------
// Reads a 32-bit pointer stored at `address`. GTA SA is a 32-bit process, so
// every pointer in the game is a 32-bit value.
uintptr_t ReadPointer(uintptr_t address);
bool WritePointer(uintptr_t address, uintptr_t value);

// Convenience read/write helpers for the most common types.
uint8_t ReadU8(uintptr_t address);
uint16_t ReadU16(uintptr_t address);
uint32_t ReadU32(uintptr_t address);
int32_t ReadI32(uintptr_t address);
float ReadFloat(uintptr_t address);

bool WriteU8(uintptr_t address, uint8_t value);
bool WriteU16(uintptr_t address, uint16_t value);
bool WriteU32(uintptr_t address, uint32_t value);
bool WriteI32(uintptr_t address, int32_t value);
bool WriteFloat(uintptr_t address, float value);

// CVector helpers ---------------------------------------------------------
struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// Reads the world position out of a CPed / CVehicle. Both use the same
// layout: a pointer to a matrix at +0x14 whose +0x30 holds the position.
bool ReadEntityPosition(uintptr_t entity, Vec3* out);
bool WriteEntityPosition(uintptr_t entity, const Vec3& pos);

// Writes the full 3x3 orientation plus the position. Used when teleporting
// entities so they do not spin wildly afterwards.
bool WriteEntityTransform(uintptr_t entity, const Vec3& pos, float headingRad);

float HeadingToDegrees(float radians);
float DegreesToHeading(float degrees);

// Writes a NUL terminated string into the game text box (0xBAA7A0).
void ShowMessage(const std::string& text);

}  // namespace sa
