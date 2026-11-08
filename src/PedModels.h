// PedModels.h - Ped model ids (from peds.ide / default.dat) and their names.
//
// The ids are the model indices used by CEntity::SetModelIndex() and stored
// in CEntity::m_wModelIndex (CPed + 0x22). Ids 0-312 are the stock ped
// models of GTA:SA v1.0.

#pragma once

#include <cstdint>
#include <string>

namespace sa {

struct PedModel {
    uint16_t id;
    const char* name;
    const char* group;  // "Civilian", "Gang", "Police", "Story", ...
};

// All ped models that can be spawned from the menu.
const PedModel* GetPedModels();
int GetPedModelCount();

// Returns the model entry for an id, or nullptr when unknown.
const PedModel* FindPedModel(uint16_t id);

// A short display name for a ped model id.
std::string PedModelName(uint16_t id);

}  // namespace sa
