#include <Windows.h>
#include <psapi.h>

#include <loguru/loguru.cpp>
#include <loguru/loguru.hpp>
#include <string>
#include <thread>

#include "core.h"

#define INJECTOR_EXE_NAME "OverlayInjector.exe"
#define INJECTOR64_EXE_NAME "OverlayInjector64.exe"

bool IsLoadedByInjector() {
  CHAR executable_name[MAX_PATH];

  // Get the executable name
  GetModuleFileNameA(NULL, executable_name, MAX_PATH);

  return std::string(executable_name).find(INJECTOR_EXE_NAME) !=
             std::string::npos ||
         std::string(executable_name).find(INJECTOR64_EXE_NAME) !=
             std::string::npos;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  // If the library was loaded by the injector, do nothing
  if (IsLoadedByInjector()) {
    return true;
  }

  // If the process was just attached to this library
  if (fdwReason == DLL_PROCESS_ATTACH) {
    // Load the library in the target process (DLL Injection)
    TCHAR dllName[MAX_PATH];
    GetModuleFileName(hinstDLL, dllName, MAX_PATH);
    LoadLibrary(dllName);

#ifdef DEBUG
    // Create debug console and redirect stdin, stdout and stderr to it
    AllocConsole();
    SetConsoleTitleW(L"Debug Console");
    freopen("conin$", "r+t", stdin);
    freopen("conout$", "w+t", stdout);
    freopen("conout$", "w+t", stderr);

    loguru::set_thread_name("application main");
#endif

    // Disable calling DllMain for DLL_THREAD_ATTACH and DLL_THREAD_DETACH
    DisableThreadLibraryCalls((HMODULE)hinstDLL);

#ifdef OVERLAY_VERSION
    DLOG_F(INFO, "Overlay Core v%s (PID: %d)\n", OVERLAY_VERSION, GetCurrentProcessId());
#else
    DLOG_F(INFO, "Overlay Core (PID: %d)\n", GetCurrentProcessId());
#endif

    // Start overlay core
    overlay::core::Core::Get()->Start(hinstDLL);
  }

  return true;
}