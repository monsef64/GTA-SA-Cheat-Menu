// VehicleModels.h - Vehicle model ids (from vehicles.ide) and their names.
//
// The ids follow the layout of data/vehicles.ide of GTA:SA v1.0: ids 400-611
// are the vehicles. Within that range the ids are interleaved between cars,
// bikes, planes, boats, trains and industrial/service vehicles, so the
// category is stored per entry rather than derived from the id.

#pragma once

#include <cstdint>
#include <string>

namespace sa {

struct VehicleModel {
    uint16_t id;
    const char* name;
    const char* category;  // "Car", "Bike", "Plane", "Boat", "Industrial", ...
};

// The spawner menu lists vehicles by category.
enum class VehicleCategory { Car, Bike, Plane, Boat, Industrial, Special };

const char* CategoryName(VehicleCategory category);

// All vehicles that can be spawned from the menu.
const VehicleModel* GetVehicleModels();
int GetVehicleModelCount();

// Returns the model entry for an id, or nullptr when unknown.
const VehicleModel* FindVehicleModel(uint16_t id);

// A short display name for a vehicle id.
std::string VehicleModelName(uint16_t id);

}  // namespace sa
