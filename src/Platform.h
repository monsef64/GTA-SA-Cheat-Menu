// Platform.h - Small platform abstraction so the code compiles with both
// MSVC (the real target) and GCC/Clang (used for the syntax checks).

#pragma once

#if defined(_MSC_VER)
#include <Windows.h>
#else
// The non-Windows build only exists so the sources can be syntax checked.
#define SA_CALL __attribute__((cdecl))
#define SA_THISCALL __attribute__((thiscall))
#endif

#if defined(_MSC_VER)
#define SA_CDECL __cdecl
#define SA_THISCALLFN __thiscall
#else
#define SA_CDECL SA_CALL
#define SA_THISCALLFN SA_THISCALL
#endif
