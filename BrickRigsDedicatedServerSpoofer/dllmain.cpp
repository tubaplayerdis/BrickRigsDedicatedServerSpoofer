// dllmain.cpp : Defines the entry point for the DLL application.
#include <windows.h>
#include <BR-SDK.hpp>
#include "SteamSubsystem.hpp"
#include "ISteamGameServer.hpp"
//#ifdef _DEBUG
#define CONSOLE
//#endif
#define ACTUAL_DEDICATED


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
        std::cout << "testing!" << std::endl;
        NewSessionSettings->bIsDedicated = true;
#ifdef ACTUAL_DEDICATED
        NewSessionSettings->bIsLANMatch = false;
        NewSessionSettings->bUseLobbiesIfAvailable = false;
        NewSessionSettings->bUsesPresence = false;
        NewSessionSettings->bShouldAdvertise = true;
        std::cout << This->SteamSubsystem->GameServerGamePort << std::endl;//Registered as 7777 question mark
        std::cout << This->SteamSubsystem->GameServerQueryPort << std::endl;//Registered as 27015
        This->SteamSubsystem->GameServerGamePort = 7777;
        This->SteamSubsystem->GameServerQueryPort = 27015;

        if (IsInit == false/*!*(&This->SteamSubsystem->bTickerStarted + 9)*/)//Somehow representative of bSteamworksGameServerInitialized;
        {
            std::cout << "SEV: " << InitSteamworksServer(This->SteamSubsystem) << std::endl;
            std::cout << *(&This->SteamSubsystem->bTickerStarted + 9) << std::endl;
            *(&This->SteamSubsystem->bTickerStarted + 9) = true;
            IsInit = true;
        }
#endif
        
        bool ret = CreateSessionHook.CallOriginal(This, HostingPlayerNum, SessionName, NewSessionSettings);
        std::cout << "Registered: " << ret << std::endl;
        Session = This;
        return ret;
    });

Hook<void(FOnlineAsyncTaskSteamCreateServer*)> FOnlineAsyncTaskSteamCreateServer_TickHook("40 55 56 48 83 EC 48 48",
    [](FOnlineAsyncTaskSteamCreateServer* This) -> void
    {
        FOnlineAsyncTaskSteamCreateServer_TickHook.CallOriginal(This);

        if (Session)
        {
            if (Session->GameServerSteamId.Object)
            {
                std::cout << "I: " << Session->GameServerSteamId.Object->UniqueNetId << std::endl;
            }
            std::cout << "O: " << Session->bPolicyResponseReceived << " " << Session->bSteamworksGameServerConnected << std::endl;
        }
        std::cout << This->bInit << " " << This->bIsComplete.Get() << " " << This->bWasSuccessful.Get() << std::endl;
        if (This->bInit && This->bIsComplete.Get() && This->bWasSuccessful.Get())
        {
            std::cout << "Connected and regisered with dedicated servers!\n";
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

DWORD WINAPI MainThread(LPVOID lpReserved)
{
    HMODULE hModule = static_cast<HMODULE>(lpReserved);
    self = hModule;

#ifdef CONSOLE //If in debug version enable console.
    AllocConsole();
    freopen_s(&pStdIn, "CONIN$", "r", stdin);
    freopen_s(&pStdOut, "CONOUT$", "w", stdout);
    freopen_s(&pStdErr, "CONOUT$", "w", stderr);
    SetConsoleTitleW(L"Brick Rigs Dedicated Server Spoofer");
    SetConsoleOutputCP(CP_UTF8);
#endif // _DEBUG

#ifdef CONSOLE
    std::cout << "Brick Rigs Dedicated Server Spoofer - American_Stig (tbgit) @Discord" << std::endl;
    //std::cout << "Press F7 to uninject" << std::endl;
#endif

    MH_Initialize(); //Initalize MinHook
    //BR_SDK_Init(); //Not needed atm

    CreateSessionHook.Create();
    CreateSessionHook.Enable();

    FOnlineAsyncTaskSteamCreateServer_TickHook.Create();
    FOnlineAsyncTaskSteamCreateServer_TickHook.Enable();

    return 0;
    
#ifdef CONSOLE
    while (true)
    {
        if (GetAsyncKeyState(VK_F6) & 0x8000)
        {
            if (Session) std::cout << Session->bSteamworksGameServerConnected << std::endl;
            if (Session && Session->GameServerSteamId.Object)
            {
                std::cout << Session->GameServerSteamId.Object->UniqueNetId << std::endl;
            }
        }

        if (GetAsyncKeyState(VK_F7) & 0x8000)
        {
            CreateThread(nullptr, 0, UnloadThread, nullptr, 0, nullptr);
            return 0;
        }

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

#ifdef CONSOLE
    fclose(pStdIn);
    fclose(pStdOut);
    fclose(pStdErr);
    SetStdHandle(STD_INPUT_HANDLE, nullptr);
    SetStdHandle(STD_OUTPUT_HANDLE, nullptr);
    SetStdHandle(STD_ERROR_HANDLE, nullptr);
    FreeConsole();
    PostMessage(GetConsoleWindow(), WM_CLOSE, 0, 0);
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

