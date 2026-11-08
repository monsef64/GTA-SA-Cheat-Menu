#include "Esp.h"

#include <d3d9.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "imgui.h"

#include "Addresses.h"
#include "D3D9Math.h"
#include "Entities.h"
#include "Hook.h"
#include "Memory.h"
#include "VehicleModels.h"

namespace sa {
namespace {

// A cached entity that is drawn by the overlay.
struct EspEntity {
    uintptr_t address = 0;
    Vec3 position{};
    float distance = 0.0f;
    uint32_t color = 0xFFFFFFFF;
    uint32_t label = 0;  // index into the label pool
    float health = 0.0f;
    bool isVehicle = false;
    bool showHealth = false;
};

// A short string shown next to the marker.
struct Label {
    std::string text;
};

std::vector<EspEntity> g_entities;
std::vector<Label> g_labels;
Vec3 g_playerPosition{};
bool g_havePlayer = false;

float DistanceBetween(const Vec3& a, const Vec3& b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// Projects a world position into screen space using the active D3D9 view and
// projection matrices. This is the same maths the game itself uses to draw
// the world, so the result lines up with what is on screen.
bool WorldToScreen(IDirect3DDevice9* device, const Vec3& world, float* outX, float* outY) {
    if (device == nullptr) return false;

    D3DVIEWPORT9 viewport;
    if (FAILED(device->GetViewport(&viewport))) return false;

    Matrix4x4 view;
    Matrix4x4 projection;
    if (FAILED(GetTransformMatrix(device, D3DTS_VIEW, &view))) return false;
    if (FAILED(GetTransformMatrix(device, D3DTS_PROJECTION, &projection))) return false;

    const Vector3 screen =
        ProjectPoint({world.x, world.y, world.z}, viewport, projection, view);

    // z outside [0, 1] means the point is behind the camera or clipped.
    if (screen.z < 0.0f || screen.z > 1.0f) return false;

    *outX = screen.x;
    *outY = screen.y;
    return true;
}

// Line drawing on the ImGui draw list.
//
// Everything the overlay draws goes through ImGui's draw list rather than
// through raw D3D9 calls. ImGui sets up its own render state when it renders
// the frame, so issuing D3D9 draw calls in the middle of an ImGui frame
// fights with that state and makes the markers flicker.
void DrawLine(ImDrawList* drawList, float x1, float y1, float x2, float y2,
              uint32_t color) {
    if (drawList == nullptr) return;

    // ImGui expects the colour as 0xAABBGGRR.
    const ImU32 abgr = (color & 0x00FFFFFF) | ((color >> 24) << 24);
    drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), abgr);
}

void DrawRectangle(ImDrawList* drawList, float x, float y, float w, float h,
                   uint32_t color) {
    if (drawList == nullptr) return;

    // ImGui expects the colour as 0xAABBGGRR.
    const ImU32 abgr = (color & 0x00FFFFFF) | ((color >> 24) << 24);
    drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), abgr);
}

uint32_t AddLabel(const std::string& text) {
    g_labels.push_back(Label{text});
    return static_cast<uint32_t>(g_labels.size() - 1);
}

// Renders a short text label next to a marker using ImGui's draw list, so
// the ESP can show names and distances.
void DrawLabel(const std::string& text, float x, float y, uint32_t color) {
    if (text.empty()) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    if (drawList == nullptr) return;

    // ImGui expects the colour as 0xAABBGGRR.
    const ImU32 abgr = (color & 0x00FFFFFF) | ((color >> 24) << 24);
    drawList->AddText(ImVec2(x, y), abgr, text.c_str());
}

}  // namespace

EspSettings& GetEspSettings() {
    static EspSettings settings;
    return settings;
}

bool IsEspEnabled() {
    return GetEspSettings().enabled;
}

void ResetEspCache() {
    g_entities.clear();
    g_labels.clear();
    g_havePlayer = false;
}

void UpdateEsp() {
    g_entities.clear();
    g_labels.clear();

    const EspSettings& settings = GetEspSettings();
    if (!settings.enabled || !IsAttached()) {
        g_havePlayer = false;
        return;
    }

    const uintptr_t playerPed = GetPlayerPed();
    if (playerPed == 0) {
        g_havePlayer = false;
        return;
    }
    if (!ReadEntityPosition(playerPed, &g_playerPosition)) {
        g_havePlayer = false;
        return;
    }
    g_havePlayer = true;

    // --- Peds -------------------------------------------------------------
    if (settings.showCops || settings.showGangMembers || settings.showCivilians) {
        const std::vector<Entity> peds = GetPeds();
        for (const Entity& ped : peds) {
            const float distance = DistanceBetween(ped.position, g_playerPosition);
            if (distance > settings.maxDistance) continue;

            const bool cop = IsCopPed(ped.address);
            const bool gang = IsGangPed(ped.address);
            const bool dead = IsPedDead(ped.address);

            if (dead) continue;

            if (cop && !settings.showCops) continue;
            if (gang && !settings.showGangMembers) continue;
            if (!cop && !gang && !settings.showCivilians) continue;

            EspEntity entity;
            entity.address = ped.address;
            entity.position = ped.position;
            entity.distance = distance;
            entity.isVehicle = false;

            if (cop) {
                entity.color = settings.copColor;
            } else if (gang) {
                entity.color = settings.gangColor;
            } else {
                entity.color = settings.civilianColor;
            }

            // Build the label from the parts the user asked for.
            std::string label;
            if (settings.showNames) {
                label = PedModelName(GetPedModelIndex(ped.address));
            }
            if (settings.showDistance) {
                if (!label.empty()) label += "  ";
                label += std::to_string(static_cast<int>(distance)) + "m";
            }
            entity.label = AddLabel(label);
            entity.health = ReadFloat(ped.address + kPedHealth);
            entity.showHealth = settings.showHealth;

            g_entities.push_back(entity);
        }
    }

    // --- Vehicles ---------------------------------------------------------
    if (settings.showVehicles) {
        const std::vector<Entity> vehicles = GetVehicles();
        for (const Entity& vehicle : vehicles) {
            const float distance = DistanceBetween(vehicle.position, g_playerPosition);
            if (distance > settings.maxDistance) continue;

            EspEntity entity;
            entity.address = vehicle.address;
            entity.position = vehicle.position;
            entity.distance = distance;
            entity.isVehicle = true;
            entity.color = settings.vehicleColor;

            // Build the label from the parts the user asked for.
            std::string label;
            if (settings.showNames) {
                label = VehicleModelName(GetVehicleModelId(vehicle.address));
            }
            if (settings.showDistance) {
                if (!label.empty()) label += "  ";
                label += std::to_string(static_cast<int>(distance)) + "m";
            }
            entity.label = AddLabel(label);
            entity.health = ReadFloat(vehicle.address + kVehHealth);
            entity.showHealth = false;

            g_entities.push_back(entity);
        }
    }
}

void DrawEsp() {
    const EspSettings& settings = GetEspSettings();
    if (!settings.enabled || !g_havePlayer || g_entities.empty()) return;

    IDirect3DDevice9* device = GetOverlayDevice();
    if (device == nullptr) return;

    D3DVIEWPORT9 viewport;
    if (FAILED(device->GetViewport(&viewport))) return;

    const float screenWidth = static_cast<float>(viewport.Width);
    const float screenHeight = static_cast<float>(viewport.Height);

    // The ESP draws into a transparent, full screen window so that ImGui
    // gives us a draw list to draw on.
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(screenWidth, screenHeight));
    ImGui::SetNextWindowBgAlpha(0.0f);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                                   ImGuiWindowFlags_NoInputs |
                                   ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_NoFocusOnAppearing;
    ImGui::Begin("##sa_esp_overlay", nullptr, flags);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    if (drawList != nullptr) {
        // The size of the box depends on how far away the entity is, so that
        // distant entities do not produce a wall of markers.
        auto boxSizeFor = [&](float distance) -> float {
            const float scale = std::max(4.0f, 40.0f - distance * 0.25f);
            return std::min(scale, 40.0f);
        };

        for (const EspEntity& entity : g_entities) {
            float screenX = 0.0f;
            float screenY = 0.0f;
            if (!WorldToScreen(device, entity.position, &screenX, &screenY)) continue;

            if (screenX < -100.0f || screenY < -100.0f) continue;
            if (screenX > screenWidth + 100.0f ||
                screenY > screenHeight + 100.0f) {
                continue;
            }

            const float size = boxSizeFor(entity.distance);

            if (settings.showBoxes) {
                // A small box around the marker position.
                DrawRectangle(drawList, screenX - size * 0.5f,
                              screenY - size * 0.5f, size, size, entity.color);
            }

            if (settings.showTracers) {
                // A line from the bottom of the screen to the entity.
                DrawLine(drawList, screenWidth * 0.5f, screenHeight, screenX,
                         screenY, entity.color);
            }

            if (settings.showHealth && entity.showHealth && entity.health > 0.0f) {
                // A small health bar above the marker.
                const float barWidth = size;
                const float barHeight = 2.0f;
                const float barX = screenX - barWidth * 0.5f;
                const float barY = screenY - size * 0.5f - 6.0f;
                const float ratio = std::min(1.0f, entity.health / 100.0f);

                DrawRectangle(drawList, barX, barY, barWidth, barHeight, 0xFF303030);
                DrawRectangle(drawList, barX, barY, barWidth * ratio, barHeight,
                              entity.color);
            }

            // The text label sits just above the marker.
            if ((settings.showNames || settings.showDistance) &&
                entity.label < g_labels.size()) {
                const std::string& label = g_labels[entity.label].text;
                if (!label.empty()) {
                    DrawLabel(label, screenX - size * 0.5f,
                              screenY - size * 0.5f - 18.0f, settings.textColor);
                }
            }
        }
    }

    ImGui::End();
}

}  // namespace sa
