#include "VehicleModels.h"

#include <cstddef>

namespace sa {
namespace {

// Vehicle model ids taken from the stock data/vehicles.ide of GTA:SA v1.0.
// Ids 400-611 are the vehicles; 612-614 are unused, and 615 onwards are
// scenery/pick-ups. Mods that replace vehicles keep their own ids, so only
// the stock ones are listed here.
constexpr VehicleModel kVehicles[] = {
    // --- Cars -------------------------------------------------------------
    {400, "Landstalker", "Car"},      {401, "Bravura", "Car"},
    {402, "Buffalo", "Car"},          {403, "Linerunner", "Car"},
    {404, "Perenniel", "Car"},        {405, "Sentinel", "Car"},
    {406, "Dumper", "Car"},           {407, "Firetruck", "Car"},
    {408, "Trashmaster", "Car"},      {409, "Stretch", "Car"},
    {410, "Manana", "Car"},           {411, "Infernus", "Car"},
    {412, "Voodoo", "Car"},           {413, "Pony", "Car"},
    {414, "Mule", "Car"},             {415, "Cheetah", "Car"},
    {416, "Ambulance", "Car"},        {417, "Leviathan", "Car"},
    {418, "Moonbeam", "Car"},         {419, "Esperanto", "Car"},
    {420, "Taxi", "Car"},             {421, "Washington", "Car"},
    {422, "Bobcat", "Car"},           {423, "Mr. Whoopee", "Car"},
    {424, "BF Injection", "Car"},     {425, "Hunter", "Car"},
    {426, "Premier", "Car"},          {427, "Enforcer", "Car"},
    {428, "Securicar", "Car"},        {429, "Banshee", "Car"},
    {430, "Predator", "Car"},         {431, "Bus", "Car"},
    {432, "Rhino", "Car"},            {433, "Barracks", "Car"},
    {434, "Hotknife", "Car"},         {435, "Artic1", "Car"},
    {436, "Previon", "Car"},          {437, "Coach", "Car"},
    {438, "Cabbie", "Car"},           {439, "Stallion", "Car"},
    {440, "Rumpo", "Car"},            {441, "RC Bandit", "Car"},
    {442, "Romero", "Car"},           {443, "Packer", "Car"},
    {444, "Monster", "Car"},          {445, "Admiral", "Car"},
    {446, "Squalo", "Car"},           {447, "Seaspar", "Car"},
    {448, "Pizzaboy", "Car"},         {449, "Tram", "Car"},
    {450, "Artic2", "Car"},           {451, "Turismo", "Car"},
    {452, "Speeder", "Car"},          {453, "Reefer", "Car"},
    {454, "Tropic", "Car"},           {455, "Flatbed", "Car"},
    {456, "Yankee", "Car"},           {457, "Caddy", "Car"},
    {458, "Solair", "Car"},           {459, "Topfun", "Car"},
    {460, "Skimmer", "Car"},          {461, "PCJ-600", "Bike"},
    {462, "Faggio", "Bike"},          {463, "Freeway", "Bike"},
    {464, "RC Baron", "Car"},         {465, "RC Raider", "Car"},
    {466, "Glendale", "Car"},         {467, "Oceanic", "Car"},
    {468, "Sanchez", "Bike"},         {469, "Sparrow", "Plane"},
    {470, "Patriot", "Car"},          {471, "Quad", "Bike"},
    {472, "Coastg", "Plane"},         {473, "Dinghy", "Boat"},
    {474, "Hermes", "Plane"},         {475, "Sabre", "Plane"},
    {476, "Rustler", "Plane"},        {477, "ZR-350", "Bike"},
    {478, "Walton", "Car"},           {479, "Regina", "Car"},
    {480, "Comet", "Car"},            {481, "BMX", "Bike"},
    {482, "Burrito", "Car"},          {483, "Camper", "Car"},
    {484, "Marquis", "Car"},          {485, "Baggage", "Car"},
    {486, "Dozer", "Industrial"},     {487, "Maverick", "Plane"},
    {488, "VCN Maverick", "Plane"},   {489, "Rancher", "Car"},
    {490, "FBI Rancher", "Car"},      {491, "Virgo", "Car"},
    {492, "Greenwood", "Car"},        {493, "Jetmax", "Boat"},
    {494, "Hotring", "Car"},          {495, "Sandking", "Car"},
    {496, "Blista", "Car"},           {497, "Polmav", "Plane"},
    {498, "Boxville", "Industrial"},  {499, "Benson", "Industrial"},
    {500, "Mesa", "Offroad"},         {501, "RC Goblin", "RC"},
    {502, "Hotrina", "RC"},           {503, "Hotrinb", "RC"},
    {504, "Bloodra", "RC"},           {505, "Ranchlure", "RC"},
    {506, "Super GT", "Car"},         {507, "Elegant", "Car"},
    {508, "Journey", "Industrial"},   {509, "Bike", "Bike"},
    {510, "Mountain Bike", "Bike"},   {511, "Beagle", "Plane"},
    {512, "Cropduster", "Plane"},     {513, "Stunt", "Plane"},
    {514, "Tanker", "Industrial"},    {515, "Roadtrain", "Industrial"},
    {516, "Nebula", "Car"},           {517, "Majestic", "Car"},
    {518, "Buccaneer", "Car"},        {519, "Shamal", "Plane"},
    {520, "Hydra", "Plane"},          {521, "FCR-900", "Bike"},
    {522, "NRG-500", "Bike"},         {523, "HPV-1000", "Bike"},
    {524, "Cement Truck", "Industrial"}, {525, "Towtruck", "Industrial"},
    {526, "Fortune", "Car"},          {527, "Cadrona", "Car"},
    {528, "FBI Truck", "Industrial"}, {529, "Willard", "Car"},
    {530, "Forklift", "Industrial"},  {531, "Tractor", "Industrial"},
    {532, "Combine Harvester", "Industrial"}, {533, "Feltzer", "Car"},
    {534, "Remington", "Car"},        {535, "Slamvan", "Car"},
    {536, "Blade", "Car"},            {537, "Freight", "Train"},
    {538, "Brownstreak", "Train"},    {539, "Vortex", "Car"},
    {540, "Vincent", "Car"},          {541, "Bullet", "Car"},
    {542, "Clover", "Car"},           {543, "Sadler", "Car"},
    {544, "Firela", "Car"},           {545, "Hustler", "Car"},
    {546, "Intruder", "Car"},         {547, "Primo", "Car"},
    {548, "Cargobob", "Plane"},       {549, "Tampa", "Car"},
    {550, "Sunrise", "Car"},          {551, "Merit", "Car"},
    {552, "Utility", "Car"},          {553, "Nevada", "Car"},
    {554, "Yosemite", "Car"},         {555, "Windsor", "Car"},
    {556, "Monster A", "Offroad"},    {557, "Monster B", "Offroad"},
    {558, "Uranus", "Car"},           {559, "Jester", "Car"},
    {560, "Sultan", "Car"},           {561, "Stratum", "Car"},
    {562, "Elegy", "Car"},            {563, "Raindance", "Car"},
    {564, "RC Tiger", "RC"},          {565, "Flash", "Car"},
    {566, "Tahoma", "Car"},           {567, "Savanna", "Car"},
    {568, "Bandito", "Car"},          {569, "Freeway Flatbed", "Car"},
    {570, "RC Bandito", "RC"},        {571, "Kart", "Car"},
    {572, "Mower", "Industrial"},     {573, "Duneride", "Offroad"},
    {574, "Sweeper", "Industrial"},   {575, "Broadway", "Car"},
    {576, "Tornado", "Car"},          {577, "AT-400", "Helicopter"},
    {578, "DFT-30", "Industrial"},    {579, "Huntley", "Offroad"},
    {580, "Stafford", "Car"},         {581, "BF-400", "Bike"},
    {582, "Newsvan", "Industrial"},   {583, "Tug", "Industrial"},
    {584, "Petrol Truck", "Industrial"}, {585, "Emperor", "Car"},
    {586, "Wayfarer", "Car"},         {587, "Euros", "Car"},
    {588, "Hotdog", "Bike"},          {589, "Club", "Car"},
    {590, "Freebox", "Industrial"},   {591, "Artic3", "Industrial"},
    {592, "Androm", "Plane"},         {593, "Dodo", "Plane"},
    {594, "RC Camera", "RC"},        {595, "Launch", "Boat"},
    {596, "Cop Car (LA)", "Car"},     {597, "Cop Car (SF)", "Car"},
    {598, "Cop Car (Vegas)", "Car"},  {599, "Cop Ranger", "Car"},
    {600, "Picador", "Industrial"},   {601, "SWAT Van", "Industrial"},
    {602, "Alpha", "Car"},            {603, "Phoenix", "Car"},
    {604, "Glenshit", "Car"},         {605, "Sadls-hit", "Car"},
    {606, "Bagbox A", "Industrial"},  {607, "Bagbox B", "Industrial"},
    {608, "Tug Stair", "Industrial"}, {609, "Boxburg", "Industrial"},
    {610, "Farmtruck", "Industrial"}, {611, "Utility Truck", "Industrial"},
};

}  // namespace

const VehicleModel* FindVehicleModel(uint16_t modelId) {
    for (const VehicleModel& model : kVehicles) {
        if (model.id == modelId) return &model;
    }
    return nullptr;
}

std::string VehicleModelName(uint16_t modelId) {
    const VehicleModel* model = FindVehicleModel(modelId);
    return model ? std::string(model->name) : std::string("Unknown");
}

const VehicleModel* GetVehicleModels() {
    return kVehicles;
}

int GetVehicleModelCount() {
    return static_cast<int>(sizeof(kVehicles) / sizeof(kVehicles[0]));
}

const char* CategoryName(VehicleCategory category) {
    switch (category) {
        case VehicleCategory::Car:       return "Cars";
        case VehicleCategory::Bike:      return "Bikes";
        case VehicleCategory::Plane:     return "Planes";
        case VehicleCategory::Boat:      return "Boats";
        case VehicleCategory::Industrial:return "Industrial";
        case VehicleCategory::Special:   return "Special";
    }
    return "Other";
}

}  // namespace sa
