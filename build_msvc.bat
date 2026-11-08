@echo off
REM ===========================================================================
REM  GTA: San Andreas ImGui Cheat Menu - MSVC build script
REM
REM  Builds a 32-bit DLL that can be injected into GTA_SA.EXE.
REM
REM  Requirements:
REM    - Visual Studio 2019/2022 with the "Desktop development with C++" workload
REM    - The Windows 10 SDK (included with Visual Studio)
REM
REM  Note: no DirectX SDK is needed. The world-to-screen projection is
REM  implemented in src\D3D9Math.h, so only d3d9.lib is linked.
REM
REM  Usage:  build_msvc.bat
REM ===========================================================================

setlocal enabledelayedexpansion

echo ============================================================
echo   GTA San Andreas - ImGui Cheat Menu  (MSVC build)
echo ============================================================
echo.

REM --- Locate vcvars32.bat -------------------------------------------------
set "VCPATH="
for %%p in (
    "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build"
    "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build"
    "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build"
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build"
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build"
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build"
) do (
    if exist "%%~p\vcvars32.bat" (
        set "VCPATH=%%~p"
    )
)

if "%VCPATH%"=="" (
    echo [ERROR] Could not find vcvars32.bat.
    echo         Install Visual Studio with the C++ desktop workload.
    exit /b 1
)

echo Using Visual Studio at: %VCPATH%
call "%VCPATH%\vcvars32.bat" >nul
if errorlevel 1 (
    echo [ERROR] Failed to initialise the Visual Studio environment.
    exit /b 1
)

REM --- Build ---------------------------------------------------------------
echo.
echo Building SAImGuiMenu.dll ...
echo.

if not exist build mkdir build

cl /nologo /LD /O2 /MT /W3 /EHsc /std:c++17 ^
   /D_CRT_SECURE_NO_WARNINGS /DWIN32_LEAN_AND_MEAN /DNOMINMAX ^
   /I"src" /I"third_party\imgui" /I"third_party\minhook\include" ^
   src\*.cpp ^
   third_party\imgui\imgui.cpp ^
   third_party\imgui\imgui_draw.cpp ^
   third_party\imgui\imgui_tables.cpp ^
   third_party\imgui\imgui_widgets.cpp ^
   third_party\imgui\imgui_impl_dx9.cpp ^
   third_party\minhook\src\buffer.c ^
   third_party\minhook\src\hook.c ^
   third_party\minhook\src\trampoline.c ^
   /link /OUT:build\SAImGuiMenu.dll d3d9.lib user32.lib gdi32.lib shell32.lib

if errorlevel 1 (
    echo.
    echo [ERROR] Build failed.
    exit /b 1
)

echo.
echo ============================================================
echo   Build succeeded: build\SAImGuiMenu.dll
echo ============================================================
echo.
echo The DLL has no export, so it must be injected with a tool that
echo supports "LoadLibrary" injection, for example:
echo.
echo   * Extreme Injector
echo   * Process Hacker  ^(Dlls ^-^> Inject DLL^)
echo   * The included tools\Injector.cpp ^(see README^)
echo.
echo Press INSERT inside the game to toggle the menu.
echo.

endlocal
