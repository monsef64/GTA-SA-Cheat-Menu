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

### How the overlay gets its input

The overlay renders ImGui directly onto the game's own Direct3D 9 device
instead of creating a window of its own, so there is no ImGui Win32
platform backend involved. Everything a platform backend would normally
provide is filled in by hand in `src/Hook.cpp`:

- `io.DisplaySize` is taken from the D3D9 viewport every frame, so the
  overlay always matches the back buffer and follows a resolution change
  on its own.
- `io.DeltaTime` falls back to 1/60 when the game has not supplied one.
- Mouse movement, the three mouse buttons, the wheel, key presses and
  character input are forwarded from the game's window procedure with
  ImGui's event API (`AddMousePosEvent`, `AddMouseButtonEvent`,
  `AddMouseWheelEvent`, `AddKeyEvent`, `AddInputCharacter`).

Input is only consumed while the menu is open, and only when ImGui
actually asks for it, so the game keeps its normal controls otherwise.
**INSERT** is reserved for toggling the menu and never reaches the game.

Because the game grabs the keyboard and the mouse, two extra measures are
needed:

- The window handle is looked up with two levels of indirection
  (`0xC17054` -> address of the handle -> handle), because the game stores
  a *pointer* to its window handle. If the handle cannot be read, the
  window is found by enumerating the top level windows of the process.
- While the menu is open the cursor is unclipped and made visible every
  frame, because the game hides and clips it again on every frame it
  renders. The cursor position is left alone so ImGui does not see the
  mouse jump around. When the menu is closed the cursor is handed back to
  the game.

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

## Troubleshooting

**`Assertion failed! ... Invalid DisplaySize value!`**

This assertion comes from `third_party/imgui/imgui.cpp` and means ImGui
was asked to start a frame without knowing how large the screen is. In
this project that happens when `io.DisplaySize` is left at `(0, 0)`,
which makes the DX9 backend build a zero sized viewport.

`UpdateImGuiFrameState()` in `src/Hook.cpp` sets it from the D3D9
viewport before every frame, so this should not happen with a current
build. If you still see it, the DLL was built from an older revision —
rebuild it from the latest source and make sure the injected DLL is the
one you just built.

**The menu shows up but clicking does nothing**

Two separate things have to work for the menu to be usable:

1. The overlay's window procedure has to be installed on the window that
   actually receives the mouse input. The game keeps a *pointer* to its
   window handle at `0xC17054`, so the handle has to be looked up with two
   levels of indirection. If only one level is read, the hook is installed
   on an address that is not a window and silently does nothing.
2. The game hides, clips and warps the mouse cursor while the player is in
   control. While the menu is open the overlay unclips the cursor and makes
   it visible again, otherwise ImGui never sees a mouse position that moves
   and nothing can be clicked.

Older builds got both of these wrong, so the menu was completely
unresponsive. Rebuild from the latest source if you see this.

**The menu does not appear at all**

Check that the DLL actually loaded (e.g. with Process Hacker).

**INSERT does not toggle the menu**

The game grabs the keyboard and swallows the key press. The overlay
therefore polls the key from the render thread instead of relying on the
window procedure, so the toggle works even when the game has focus.

---

## Credits & licence

- Memory addresses and function addresses: [GTAMods wiki](https://gtamods.com/wiki/Memory_Addresses_(SA))
- [Dear ImGui](https://github.com/ocornut/imgui) — MIT licence
- [MinHook](https://github.com/TsudaKageyu/minhook) — BSD 2-Clause licence

This project is released under the **GNU GPL v3** (see `LICENSE`).

This is a single-player modification tool for a game you own. It is not
intended for online play.
