cmake_minimum_required(VERSION 3.15)
project(GtaSaCheatMenu LANGUAGES C CXX)

# The game is a 32-bit process, so the cheat menu has to be built for x86.
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR
        "GTA: San Andreas is a 32-bit game. Configure with -A Win32 or use the "
        "provided build_msvc.bat.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SA_MENU_SOURCES
    src/DllMain.cpp
    src/Hook.cpp
    src/Memory.cpp
    src/Entities.cpp
    src/Esp.cpp
    src/Cheats.cpp
    src/Spawner.cpp
    src/Menu.cpp
    src/VehicleModels.cpp
    src/PedModels.cpp
    src/Weapons.cpp
)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)
set(MINHOOK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/minhook)

add_library(SAImGui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_dx9.cpp
)
target_include_directories(SAImGui PUBLIC ${IMGUI_DIR})

add_library(SAMinHook STATIC
    ${MINHOOK_DIR}/src/buffer.c
    ${MINHOOK_DIR}/src/hook.c
    ${MINHOOK_DIR}/src/trampoline.c
    # The Hacker Disassembler Engine. MinHook's trampoline calls
    # hde32_disasm / hde64_disasm, so these have to be compiled in or the
    # link fails with an unresolved external.
    ${MINHOOK_DIR}/src/hde/hde32.c
    ${MINHOOK_DIR}/src/hde/hde64.c
)
target_include_directories(SAMinHook PUBLIC ${MINHOOK_DIR}/include)

add_library(SAImGuiMenu SHARED ${SA_MENU_SOURCES})
target_include_directories(SAImGuiMenu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(SAImGuiMenu PRIVATE SAImGui SAMinHook d3d9)

set_target_properties(SAImGuiMenu PROPERTIES
    OUTPUT_NAME "SAImGuiMenu"
    PREFIX ""
)
