# GTA: San Andreas — ImGui Cheat Menu

A DirectX 9 + Dear ImGui cheat menu for **GTA: San Andreas**, built around the
memory addresses published on the
[GTAMods wiki](https://gtamods.com/wiki/Memory_Addresses_(SA)) and
[Function Memory Addresses (SA)](https://gtamods.com/wiki/Function_Memory_Addresses_(SA)).

The menu is injected into the running game as a DLL. It hooks the game's D3D9
`EndScene` / `Reset` vtable entries with [MinHook](https://github.com/TsudaKageyu/minhook)
and renders an [ImGui](https://github.com/ocornut/imgui) window on top of the
game, plus a separate world-space overlay for the ESP.

---

## Features

### Enemy ESP
- Marks police / SWAT / FBI / army peds, gang members, civilians and vehicles.
- Draws a marker box, an optional tracer line from the bottom of the screen and
  a health bar over every target.
- Colour-coded per category (blue = police, green = gangs, grey = civilians,
  amber = vehicles), all configurable.
- Distance limit slider, optional "through walls" mode.
- Projection uses the game's own D3D9 view/projection matrices
  (`D3DXVec3Project`), so markers line up exactly with the rendered world.

### Player
- Set / add money (up to `999,999,999`).
- Set health and armour, instant "god mode" via the game's own cheat byte.
- Max out every skill and stat (driving, flying, bike, cycling, stamina,
  muscle, respect, sex appeal…).
- Set and clear the wanted level.
- Repair the current vehicle, refill nitro, fix flat tyres.

### Weapon spawner
- Give yourself any weapon from the full `eWeaponType` list (fists through
  minigun), with a configurable ammo count.
- "Give All Weapons" fills every weapon slot at once.
- "Remove Weapons" strips the player back to fists.
- The original game's *Weapon Set 1 / 2 / 3* cheat sets.

### Vehicle spawner
- Spawn any stock vehicle from `vehicles.ide` in front of you. The full
  stock table (ids **400-611**) is browsable by category: cars, bikes, planes,
  boats, trains, industrial, off-road, RC and the helicopter.
- Spawn the cheat-only vehicles too (Rhino, Hunter, Hydra, Vortex Hovercraft,
  Dozer, Tanker, Stunt Plane, Quad, Monster) via the game's own cheat
  routines.
- A custom model-id field lets you spawn any vehicle id, including ones added
  by mods.
- Delete the vehicles you spawned, or every vehicle in the world.
- Traffic density slider (0× – 5×).

### Ped spawner
- Spawn any ped model next to you. The full stock table (ids **0-312**) is
  browsable by group: civilians, gang members, police (city cop, LAPD, SFPD,
  SWAT, FBI, army, sheriffs) and the story characters.
- Peds are built with the game's own `CPed` constructor and
  `CEntity::SetModelIndex()`, so they get a valid vtable, an intelligence
  block and a task manager — they walk around, take cover and fight.
- Optionally spawn them armed with a weapon of your choice.
- Kill all peds, or delete only the ones you spawned.
- Ped density slider (0× – 5×).

### Cheats
- ~45 of the original game's cheat switches, exposed as checkboxes:
  infinite health/oxygen, jetpack, parachute, mega punch, never get hungry,
  max respect / sex appeal / stamina, peds riot, gangs control the streets,
  gang members everywhere, peds attack with golf clubs / rockets, Elvis
  everywhere, cars can fly, boats can fly, cars drive on water, tank mode,
  all cars have nitro, cheap/fast traffic, aggressive drivers, invisible
  (wheels only) cars, perfect handling, huge bunny hop, mega jump, full aiming
  while driving, lock wanted level, six-star wanted level, hitman in all
  weapons, and more.
- One-click "Reset All Cheats".

### Misc
- Weather selection (all 23 named weather types plus the numbered extras).
- Set and freeze the in-game clock.
- HUD / radar / subtitle toggles, brightness, and game-speed control.

---

## Building

The game is a **32-bit** process, so the DLL **must** be built for **Win32**.

### GitHub Actions (recommended)

The repository has a workflow at
[`.github/workflows/build.yml`](.github/workflows/build.yml) that builds the
DLL with **MSVC** on a Windows runner — no local toolchain needed.

* It runs on every push and pull request (and on demand via
  *Workflow dispatch*).
* The **Release** job produces `SAImGuiMenu.dll` (x86) plus the optional
  `Injector.exe`; the **Debug** job produces `SAImGuiMenu-debug.dll`.
* Both are uploaded as build artifacts. Download them from the **Actions**
  tab of the repository → pick a run → **Artifacts** → unzip.
* The workflow also asserts that the produced DLL really is a 32-bit PE
  image, so a misconfigured build fails instead of shipping a DLL the game
  cannot load.

To get the artifacts without touching GitHub, use one of the local options
below.

### Visual Studio

1. Open `build_msvc/SAImGuiMenu.sln`.
2. Make sure the platform is **Win32** (not x64) and the configuration is
   **Release**.
3. Build. The DLL is written to `build/Win32/Release/SAImGuiMenu.dll`.

### Command line (MSVC)

```
build_msvc.bat
```

This locates `vcvars32.bat`, compiles everything and drops the DLL into
`build/`.

### CMake

```
cmake -B build -A Win32
cmake --build build --config Release
```

> **No DirectX SDK is required.** The world-to-screen projection is
> implemented in `src/D3D9Math.h` and only `d3d9.lib` is linked. (It used to
> call `D3DXVec3Project`, which needs the old DirectX SDK from June 2010;
> the replacement is a faithful reimplementation that was verified against
> the reference algorithm to within float rounding.)

---

## Injecting

The DLL is a normal injectable library — it does **not** contain its own
injector. Use whichever tool you prefer:

- **Extreme Injector** — open `SAImGuiMenu.dll`, pick `gta_sa.exe`, Inject.
- **Process Hacker** — kill the process, use *Inject DLL*.
- A **manual mapper** if you need a stealthier approach.

After injecting, press **INSERT** to toggle the menu.

> `gta_sa.exe` must be running **as administrator** if the game is installed
> in `Program Files`, otherwise the DLL will not load.

---

## Project layout

```
src/
  Addresses.h      All memory / function addresses from the wiki
  Platform.h       Calling-convention shim (MSVC vs GCC)
  Memory.h/.cpp    Safe read/write helpers, vector maths, game text box
  Entities.h/.cpp  Ped & vehicle pool walking, entity classification
  D3D9Math.h       World-to-screen projection (replaces d3dx9.h)
  Esp.h/.cpp       World-to-screen projection and overlay drawing
  Cheats.h/.cpp    The cheat functions the menu calls
  Spawner.h/.cpp   Vehicle / ped creation and deletion
  VehicleModels.h/.cpp  Vehicle model table (vehicles.ide, ids 400-611)
  PedModels.h/.cpp      Ped model table (peds.ide, ids 0-312)
  Weapons.h/.cpp   Weapon table and slot mapping
  Menu.h/.cpp      The ImGui interface
  Hook.h/.cpp      D3D9 vtable hooks, window procedure, ImGui lifecycle
  DllMain.cpp      Entry point
third_party/
  imgui/           Dear ImGui v1.90.9 (MIT)
  minhook/         MinHook (BSD 2-clause)
build_msvc/        Visual Studio solution + project
.github/workflows/build.yml   MSVC x86 build on GitHub Actions
tools/             Helper scripts (Injector.cpp)
```

---

## Compatibility

| Game version | Supported? |
| --- | --- |
| GTA_SA.EXE **v1.0** (original retail, 32-bit) | ✅ Yes |
| v1.01 | ⚠️ Mostly — some addresses moved |
| Steam **v2.0 / v3.0** (64-bit) | ❌ No |

The v2.0+ Steam release re-laid out the executable, so **every** address in
this project is invalid there. The menu is built for the original 32-bit
`GTA_SA.EXE` v1.0.

Using the menu also sets the game's *"has ever cheated"* flag when you use the
original cheat switches, so the game will warn you when saving.

---

## Credits & licence

- Memory addresses and function addresses: [GTAMods wiki](https://gtamods.com/wiki/Memory_Addresses_(SA))
- [Dear ImGui](https://github.com/ocornut/imgui) — MIT licence
- [MinHook](https://github.com/TsudaKageyu/minhook) — BSD 2-Clause licence

This project is released under the **GNU GPL v3** (see `LICENSE`).

This is a single-player modification tool for a game you own. It is not
intended for online play.
