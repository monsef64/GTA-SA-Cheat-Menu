#include "Memory.h"

#include <Windows.h>

#include <cmath>
#include <cstring>

#include "Addresses.h"

namespace sa {
namespace {

uintptr_t g_base = 0;

}  // namespace

uintptr_t GameBase() { return g_base; }

bool IsAttached() { return g_base != 0; }

void SetGameBase(uintptr_t base) { g_base = base; }

// GTA:SA v1.0 is linked at a fixed base of 0x400000, so a function address
// from the wiki is used as-is. The module base is only used to sanity check
// that we really are inside the game executable.
constexpr uintptr_t kGamePreferredBase = 0x400000;

uintptr_t ResolveAddress(uintptr_t absoluteAddress) {
    if (g_base == 0) return 0;

    // Prefer the absolute wiki address when the module really is loaded at
    // the expected base, otherwise fall back to base + offset so a relocated
    // build still works.
    if (g_base == kGamePreferredBase) return absoluteAddress;

    const uintptr_t offset = absoluteAddress - kGamePreferredBase;
    return g_base + offset;
}

bool ReadBytes(uintptr_t address, void* out, size_t size) {
    if (g_base == 0 || address == 0 || out == nullptr) return false;
    std::memcpy(out, reinterpret_cast<const void*>(address), size);
    return true;
}

bool WriteBytes(uintptr_t address, const void* in, size_t size) {
    if (g_base == 0 || address == 0 || in == nullptr || size == 0) return false;

    // Most of the structures we touch live in writable heap pages, but a few
    // (the text box, some globals) sit in the read-only image section. Make
    // the page temporarily writable so a stray write cannot crash the game.
    DWORD oldProtect = 0;
    const bool needProtect =
        VirtualProtect(reinterpret_cast<void*>(address), size, PAGE_READWRITE,
                       &oldProtect) != FALSE;

    std::memcpy(reinterpret_cast<void*>(address), in, size);

    if (needProtect) {
        DWORD ignored = 0;
        VirtualProtect(reinterpret_cast<void*>(address), size, oldProtect,
                       &ignored);
    }
    return true;
}

uintptr_t ReadPointer(uintptr_t address) {
    return static_cast<uintptr_t>(ReadU32(address));
}

bool WritePointer(uintptr_t address, uintptr_t value) {
    return WriteU32(address, static_cast<uint32_t>(value));
}

uint8_t ReadU8(uintptr_t address) { return ReadOr<uint8_t>(address, 0); }
uint16_t ReadU16(uintptr_t address) { return ReadOr<uint16_t>(address, 0); }
uint32_t ReadU32(uintptr_t address) { return ReadOr<uint32_t>(address, 0); }
int32_t ReadI32(uintptr_t address) { return ReadOr<int32_t>(address, 0); }
float ReadFloat(uintptr_t address) { return ReadOr<float>(address, 0.0f); }

bool WriteU8(uintptr_t address, uint8_t value) { return Write<uint8_t>(address, value); }
bool WriteU16(uintptr_t address, uint16_t value) { return Write<uint16_t>(address, value); }
bool WriteU32(uintptr_t address, uint32_t value) { return Write<uint32_t>(address, value); }
bool WriteI32(uintptr_t address, int32_t value) { return Write<int32_t>(address, value); }
bool WriteFloat(uintptr_t address, float value) { return Write<float>(address, value); }

bool ReadEntityPosition(uintptr_t entity, Vec3* out) {
    if (entity == 0 || out == nullptr) return false;
    const uintptr_t matrix = ReadPointer(entity + kPedPosMatrix);
    if (matrix == 0) return false;
    return ReadBytes(matrix + 0x30, out, sizeof(Vec3));
}

bool WriteEntityPosition(uintptr_t entity, const Vec3& pos) {
    if (entity == 0) return false;
    const uintptr_t matrix = ReadPointer(entity + kPedPosMatrix);
    if (matrix == 0) return false;
    return WriteBytes(matrix + 0x30, &pos, sizeof(Vec3));
}

bool WriteEntityTransform(uintptr_t entity, const Vec3& pos, float headingRad) {
    if (entity == 0) return false;
    const uintptr_t matrix = ReadPointer(entity + kPedPosMatrix);
    if (matrix == 0) return false;

    // The game's matrix (CMatrix / RwMatrix) is 0x40 bytes and laid out as:
    //   +0x00 right  (the entity's local X axis)
    //   +0x10 up     (the entity's local Y axis)
    //   +0x20 at     (the entity's local Z axis, i.e. facing direction)
    //   +0x30 pos    (world position)
    //
    // GTA:SA is Z-up, so "up" stays (0,0,1) and the heading rotates the
    // right / at vectors in the XY plane.
    const float cosH = std::cos(headingRad);
    const float sinH = std::sin(headingRad);

    const Vec3 right{cosH, sinH, 0.0f};
    const Vec3 up{0.0f, 0.0f, 1.0f};
    const Vec3 at{-sinH, cosH, 0.0f};

    WriteBytes(matrix + 0x00, &right, sizeof(Vec3));
    WriteBytes(matrix + 0x10, &up, sizeof(Vec3));
    WriteBytes(matrix + 0x20, &at, sizeof(Vec3));
    WriteBytes(matrix + 0x30, &pos, sizeof(Vec3));
    return true;
}

float HeadingToDegrees(float radians) {
    float degrees = radians * (180.0f / 3.14159265358979323846f);
    if (degrees < 0.0f) degrees += 360.0f;
    return degrees;
}

float DegreesToHeading(float degrees) {
    while (degrees < 0.0f) degrees += 360.0f;
    while (degrees >= 360.0f) degrees -= 360.0f;
    return degrees * (3.14159265358979323846f / 180.0f);
}

void ShowMessage(const std::string& text) {
    if (g_base == 0) return;
    const size_t maxLen = 149;
    char buffer[150] = {0};
    const size_t len = text.size() < maxLen ? text.size() : maxLen;
    std::memcpy(buffer, text.c_str(), len);
    WriteBytes(base::kTextBox, buffer, sizeof(buffer));
}

}  // namespace sa
