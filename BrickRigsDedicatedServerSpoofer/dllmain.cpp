#include <windows.h>
#include <BR-SDK.hpp>
#include <string>
#include <iostream>
#include "MACROS.hpp"

#pragma region brickrust

#ifdef DEDICATED
#define PLUGIN_NAME_BRICKRUST "Dedicated Server Plugin (Dedicated)"
#elif HEADLESS
#define PLUGIN_NAME_BRICKRUST "Headless Server Plugin (Headless)"
#else
#define PLUGIN_NAME_BRICKRUST "Headless Server Plugin (Spoof)"
#endif

struct ModInfo
{
    const char* name;
    const char* description;
    const char* version;
    const char* game_version;
    const char* authors;
};

extern "C" {
    __declspec(dllexport) ModInfo mod_info()
    {
        return ModInfo {
            .name = PLUGIN_NAME_BRICKRUST,
            .description = "Enables dedicated server functionality for Brick Rigs",
            .version = "1.0.0",
            .game_version = "1.10.7",
            .authors = "American_Stig (tbgit) @Discord"
        };
    }

    __declspec(dllexport) void mod_init()
    {
        return;//Nothing atm
    }
}

#pragma endregion brickrust

//Forward Declarations
void SetupSessionSpooferHooks();
void SetupConsoleOutputOverride();
void SetupMainThreadExecution();
void RunOnMainThread(std::function<void()> func);

//Global variables
HMODULE self = nullptr;
FILE* pStdIn = nullptr;
FILE* pStdOut = nullptr;
FILE* pStdErr = nullptr;

//Safley unloads the mod. Delay added to help with execution.
DWORD WINAPI UnloadThread(LPVOID lpParam) {
    Sleep(10);
    FreeLibraryAndExitThread(self, 0);
}


DWORD WINAPI MainThread(LPVOID lpReserved)
{
    HMODULE hModule = static_cast<HMODULE>(lpReserved);
    self = hModule;

#ifndef SPOOF //Spoof def does not have console
#ifndef HEADLESS //Whether or not to create our own console or attach one.
    AllocConsole();
    freopen_s(&pStdIn, "CONIN$", "r", stdin);
    freopen_s(&pStdOut, "CONOUT$", "w", stdout);
    freopen_s(&pStdErr, "CONOUT$", "w", stderr);
    SetConsoleTitleA(SOFTWARE_NAME);
    SetConsoleOutputCP(CP_UTF8);
#else
    AttachConsole(GetCurrentProcessId());
    freopen_s(&pStdIn, "CONIN$", "r", stdin);
    freopen_s(&pStdOut, "CONOUT$", "w", stdout);
    freopen_s(&pStdErr, "CONOUT$", "w", stderr);
    SetConsoleOutputCP(CP_UTF8);
    std::ios::sync_with_stdio(true);
#endif
#endif

    std::cout<< LOG_FIX << "Brick Rigs Dedicated Server Plugin - American_Stig (tbgit) @Discord" << std::endl;

    MH_Initialize(); //Initalize MinHook
    BR_SDK_Init();

#ifdef HEADLESS
    SetupMainThreadExecution();
#endif

    SetupSessionSpooferHooks();
    SetupConsoleOutputOverride();

#ifdef HEADLESS
    while (true)
    {
        std::wstring command;
        std::cout<< LOG_FIX << "Enter Command: ";
        std::getline(std::wcin, command);

        static SDK::FString Command(command.c_str());
        Command = SDK::FString(command.c_str());

        static Function<void(SDK::UConsole* Console, SDK::FString* Command)> ConsoleCommand("48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 40 83 7A");

        RunOnMainThread([]() -> void
        {
            std::wcout<< LOG_FIX_W << L"Executing: " << Command.ToWString() << std::endl;
            ConsoleCommand(SDK::UEngine::GetEngine()->GameViewport->ViewportConsole, &Command);
        });

        Sleep(10);
    }
#endif

    return 0;
}

void CleanUp(HMODULE hModule)
{
    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

#ifndef SPOOF
#ifndef HEADLESS
    fclose(pStdIn);
    fclose(pStdOut);
    fclose(pStdErr);
    SetStdHandle(STD_INPUT_HANDLE, nullptr);
    SetStdHandle(STD_OUTPUT_HANDLE, nullptr);
    SetStdHandle(STD_ERROR_HANDLE, nullptr);
    FreeConsole();
    PostMessage(GetConsoleWindow(), WM_CLOSE, 0, 0);
#else
    fclose(pStdIn);
    fclose(pStdOut);
    fclose(pStdErr);
#endif
#endif
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
    }

    if (reason == DLL_PROCESS_DETACH)
    {
        CleanUp(self);
    }
    return TRUE;
}

