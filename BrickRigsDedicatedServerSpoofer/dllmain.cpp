#include <windows.h>
#include <BR-SDK.hpp>
#include "SteamSubsystem.hpp"
#include "ISteamGameServer.hpp"
#include <mutex>
#include <string>
#include <iostream>
#include "auth.hpp"

#ifdef RELEASE
#define DEDICATED
//#define HEADLESS
//#define SPOOF
#endif

//Call servers: https://api.steampowered.com/IGameServersService/GetServerList/v1/?key=0C439C917498DD49700A29AE4CF16250&filter=\appid\552100&limit=100

/*
* {"response":{"servers":[{"addr":"186.79.115.82:27015","gameport":7777,"steamid":"90292121794192404","name":"My Server #955","appid":552100,"gamedir":"BrickRigs","version":"1.0.0.2","product":"BrickRigs","region":-1,"players":0,"max_players":16,"bots":0,"map":"LI_Canyon","secure":true,"dedicated":true,"os":"w","gametype":"BUILDID:-1252925617,OWNINGID:76561198686315881,OWNINGNAME:My Server #955,SESSIONFLAGS:683,PASSWORD_i:0,ALLOWMODS_i:0,FPS_i:1014"}]}}
*/

Function<void* (FOnlineSessionSteam*)> GetGameServerSession("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8D 99 88 02");

bool InitSteamworksServer(FOnlineSubsystemSteam* This)
{
    static Function<bool(FOnlineSubsystemSteam*)> InitSteamworksServer_F("48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 48 83 EC 40 48 8B F9 48");
    return InitSteamworksServer_F(This);
}

static bool IsInit = false;

FOnlineSessionSteam* Session = nullptr;

Hook<bool(FOnlineSessionSteam*, int, SDK::FName, FOnlineSessionSettings*)> CreateSessionHook("4C 89 4C 24 20 89 54 24 10 55 53 56 57 41 54",
    [](FOnlineSessionSteam* This, int HostingPlayerNum, SDK::FName SessionName, FOnlineSessionSettings* NewSessionSettings) -> bool
    {
        NewSessionSettings->bIsDedicated = true;
#ifdef DEDICATED
        NewSessionSettings->bIsLANMatch = false;
        NewSessionSettings->bUseLobbiesIfAvailable = false;
        NewSessionSettings->bUsesPresence = false;
        NewSessionSettings->bShouldAdvertise = true;
        std::cout << This->SteamSubsystem->GameServerGamePort << std::endl;//Registered as 7777 question mark
        std::cout << This->SteamSubsystem->GameServerQueryPort << std::endl;//Registered as 27015
        This->SteamSubsystem->GameServerGamePort = 7777;
        This->SteamSubsystem->GameServerQueryPort = 27015;

        bool bSteamworksGameServerInitialized = *(&This->SteamSubsystem->bTickerStarted + 9);
        if (!IsInit)//Somehow representative of bSteamworksGameServerInitialized;
        {
            std::cout << "Init Steamworks Server!" << std::endl;
            std::cout << "Init: " << InitSteamworksServer(This->SteamSubsystem) << std::endl;
            std::cout << bSteamworksGameServerInitialized << std::endl;
            *(&This->SteamSubsystem->bTickerStarted + 9) = true;
            IsInit = true;
        }
#endif
        
        bool ret = CreateSessionHook.CallOriginal(This, HostingPlayerNum, SessionName, NewSessionSettings);
        std::cout << "Started Registration: " << ret << std::endl;
        Session = This;
        return ret;
    });

Hook<void(FOnlineAsyncTaskSteamCreateServer*)> FOnlineAsyncTaskSteamCreateServer_TickHook("40 55 56 48 83 EC 48 48",
    [](FOnlineAsyncTaskSteamCreateServer* This) -> void
    {
        FOnlineAsyncTaskSteamCreateServer_TickHook.CallOriginal(This);

        std::cout << "Init?: " << This->bInit << "\nComplete?: " << This->bIsComplete.Get() << "\nSuccessful?: " << This->bWasSuccessful.Get() << std::endl;
        if (Session)
        {
            if (Session->GameServerSteamId.Object)
            {
                std::cout << "GameServerID: " << Session->GameServerSteamId.Object->UniqueNetId << std::endl;
            }
            std::cout << "Policy?: " << Session->bPolicyResponseReceived << "\nServer Connection?: " << Session->bSteamworksGameServerConnected << std::endl;
        }
        if (This->bInit && This->bIsComplete.Get() && This->bWasSuccessful.Get())
        {
            std::cout << "Connected and registered with dedicated servers!\n";
        }
    });

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


std::mutex main_mutex;
auto MainThreadFunctions = std::vector<std::function<void()>>();

Signature FEngineLoopTick("48 8B C4 48 89 58 ?? 48 89 70 ?? 48 89 78 ?? 55 41 54 41 55 41 56 41 57 48 8D 68 ?? 48 81 EC ?? ?? ?? ?? 0F 29 70 ?? 48 8D 15 ?? ?? ?? ?? 48 8D 44 24 ??");
void HookedTick(void* EngineLoopPtr);
Hook<void(void*)> EngineLoopHook(FEngineLoopTick, HookedTick);

static std::atomic_bool ShouldUninject = false;
void HookedTick(void* EngineLoopPtr)
{
    if (ShouldUninject)
    {
        EngineLoopHook.Disable();
        CreateThread(nullptr, 0, UnloadThread, EngineLoopPtr, 0, nullptr);
        EngineLoopHook.CallOriginalFunction(EngineLoopPtr);
        return;
    }

    std::unique_lock lock(main_mutex);
    if (!MainThreadFunctions.empty())
    {
        for (const std::function<void()>& mainThreadFunction : MainThreadFunctions)
        {
            mainThreadFunction();
        }
        MainThreadFunctions.clear();
    }
    lock.unlock();

    EngineLoopHook.CallOriginalFunction(EngineLoopPtr);
}

void RunOnMainThread(std::function<void()> func)
{
    std::unique_lock lock(main_mutex);
    MainThreadFunctions.push_back(func);
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

    std::cout << "Brick Rigs Dedicated Server Plugin - American_Stig (tbgit) @Discord" << std::endl;

    MH_Initialize(); //Initalize MinHook
    BR_SDK_Init();

    EngineLoopHook.Create();
    EngineLoopHook.Enable();

#ifndef RELEASE
    if (!auth::GetAuthed())
    {
        ShouldUninject = true;
        return 0;
    }
#endif // RELEASE

    CreateSessionHook.Create();
    CreateSessionHook.Enable();

    FOnlineAsyncTaskSteamCreateServer_TickHook.Create();
    FOnlineAsyncTaskSteamCreateServer_TickHook.Enable();

#ifdef SPOOF
    return 0;
#endif

    std::cout << "Use Command: Uninject to uninject" << std::endl;
    while (true)
    {
        std::wstring command;
        std::wcout << "\nENTER COMMAND: ";
        std::getline(std::wcin, command);

        static SDK::FString Command(command.c_str());
        Command = SDK::FString(command.c_str());

        if (command == L"Uninject")
        {
            ShouldUninject = true;
            return 0;
        }

        RunOnMainThread([]() -> void
        {
            std::wcout << L"Executing: " << Command.ToWString() << std::endl;
            SDK::UKismetSystemLibrary::ExecuteConsoleCommand(SDK::UWorld::GetWorld(), Command, nullptr);
        });

        Sleep(10);
    }

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

