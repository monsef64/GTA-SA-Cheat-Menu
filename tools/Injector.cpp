// Injector.cpp - A minimal DLL injector for the cheat menu.
//
// The cheat menu DLL is a normal injectable library. This small helper loads
// it into a running gta_sa.exe using CreateRemoteThread + LoadLibraryA, which
// is the simplest approach and works on most Windows configurations.
//
// Build (32-bit, MSVC):
//   cl /nologo /EHsc /std:c++17 Injector.cpp /link /OUT:Injector.exe
//
// Usage:
//   Injector.exe                     (injects into "gta_sa.exe")
//   Injector.exe gta_sa.exe path\to\SAImGuiMenu.dll
//
// Note: the game must be running as administrator for the injection to
// succeed when it lives in Program Files.

#include <Windows.h>
#include <TlHelp32.h>

#include <cstdio>
#include <cstring>
#include <string>

namespace {

constexpr DWORD kProcessAll = PROCESS_ALL_ACCESS;

bool ProcessIdFromName(const char* name, DWORD* pidOut) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    bool found = false;
    if (Process32First(snapshot, &entry)) {
        do {
            if (_stricmp(entry.szExeFile, name) == 0) {
                *pidOut = entry.th32ProcessID;
                found = true;
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return found;
}

std::string GetCurrentDirectoryPath() {
    char buffer[MAX_PATH] = {0};
    GetCurrentDirectoryA(MAX_PATH, buffer);
    return std::string(buffer);
}

bool Inject(DWORD pid, const std::string& dllPath) {
    // Open the target process.
    HANDLE process = OpenProcess(kProcessAll, FALSE, pid);
    if (process == nullptr) {
        std::printf("[!] Could not open the process (error %lu).\n", GetLastError());
        std::printf("    Try running this injector as administrator.\n");
        return false;
    }

    // Allocate space in the target for the DLL path.
    const SIZE_T pathBytes = dllPath.size() + 1;
    void* remoteMemory = VirtualAllocEx(process, nullptr, pathBytes, MEM_COMMIT | MEM_RESERVE,
                                        PAGE_READWRITE);
    if (remoteMemory == nullptr) {
        std::printf("[!] VirtualAllocEx failed (error %lu).\n", GetLastError());
        CloseHandle(process);
        return false;
    }

    // Write the DLL path into the target process.
    SIZE_T written = 0;
    if (!WriteProcessMemory(process, remoteMemory, dllPath.c_str(), pathBytes, &written) ||
        written != pathBytes) {
        std::printf("[!] WriteProcessMemory failed (error %lu).\n", GetLastError());
        VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    // Resolve LoadLibraryA inside the target.
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    FARPROC loadLibrary = GetProcAddress(kernel32, "LoadLibraryA");
    if (loadLibrary == nullptr) {
        std::printf("[!] Could not find LoadLibraryA.\n");
        VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    // Start the remote thread.
    HANDLE thread = CreateRemoteThread(process, nullptr, 0,
                                        reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibrary),
                                        remoteMemory, 0, nullptr);
    if (thread == nullptr) {
        std::printf("[!] CreateRemoteThread failed (error %lu).\n", GetLastError());
        VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    WaitForSingleObject(thread, 5000);

    DWORD exitCode = 0;
    GetExitCodeThread(thread, &exitCode);
    CloseHandle(thread);

    VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(process);

    if (exitCode == 0) {
        std::printf("[!] LoadLibraryA returned NULL in the target process.\n");
        return false;
    }

    return true;
}

}  // namespace

int main(int argc, char** argv) {
    const char* processName = "gta_sa.exe";
    std::string dllPath;

    if (argc >= 2) {
        processName = argv[1];
    }
    if (argc >= 3) {
        dllPath = argv[2];
    } else {
        dllPath = GetCurrentDirectoryPath() + "\\SAImGuiMenu.dll";
    }

    std::printf("===========================================\n");
    std::printf("  GTA San Andreas - Cheat Menu Injector\n");
    std::printf("===========================================\n\n");

    DWORD pid = 0;
    if (!ProcessIdFromName(processName, &pid)) {
        std::printf("[!] Could not find a running process called '%s'.\n", processName);
        std::printf("    Start the game first, then run the injector again.\n");
        return 1;
    }
    std::printf("[+] Found '%s' (PID %lu).\n", processName, pid);
    std::printf("[+] Injecting: %s\n", dllPath.c_str());

    if (!Inject(pid, dllPath)) {
        std::printf("\n[!] Injection failed.\n");
        return 1;
    }

    std::printf("\n[+] Injected successfully.\n");
    std::printf("[+] Press INSERT inside the game to toggle the menu.\n");
    return 0;
}
