// Esp.h - The enemy ESP / world overlay.
//
// The ESP works by projecting world positions of peds and vehicles onto the
// screen and drawing a marker with the D3D9 device. It runs every frame from
// the render hook.

#pragma once

#include <cstdint>

namespace sa {

// Runtime options for the overlay.
struct EspSettings {
    bool enabled = false;

    // Which kinds of targets to mark.
    bool showCops = true;         // police / SWAT / FBI / army
    bool showGangMembers = true;   // Ballas / Vagos / Grove Street / ...
    bool showCivilians = false;   // everybody else
    bool showVehicles = false;

    // How the markers are drawn.
    bool showNames = true;
    bool showDistance = true;
    bool showHealth = true;
    bool showBoxes = true;
    bool showTracers = false;

    float maxDistance = 150.0f;   // meters
    bool throughWalls = true;

    // Colours (RGBA, 0-255).
    uint32_t copColor = 0xFF3060FF;       // blue
    uint32_t gangColor = 0xFF20E060;      // green
    uint32_t civilianColor = 0xFFB0B0B0;  // grey
    uint32_t vehicleColor = 0xFFFFC020;   // amber
    uint32_t textColor = 0xFFFFFFFF;      // white
};

EspSettings& GetEspSettings();

// True when the overlay should be drawn this frame.
bool IsEspEnabled();

// Updates the cached entity list. Called once per frame before drawing.
void UpdateEsp();

// Draws the overlay for the current frame.
void DrawEsp();

// Clears the cached entity list (e.g. when the menu is closed).
void ResetEspCache();

}  // namespace sa
