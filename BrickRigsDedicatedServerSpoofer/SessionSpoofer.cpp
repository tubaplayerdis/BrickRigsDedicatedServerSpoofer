#include <BR-SDK.hpp>
#include "SteamSubsystem.hpp"
#include "ISteamGameServer.hpp"
#include "MACROS.hpp"

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
        //std::cout << This->SteamSubsystem->GameServerGamePort << std::endl;//Registered as 7777 question mark
        //std::cout << This->SteamSubsystem->GameServerQueryPort << std::endl;//Registered as 27015
        This->SteamSubsystem->GameServerGamePort = 7777;
        This->SteamSubsystem->GameServerQueryPort = 27015;

        bool bSteamworksGameServerInitialized = *(&This->SteamSubsystem->bTickerStarted + 9);
        if (!IsInit)//Somehow representative of bSteamworksGameServerInitialized;
        {
            std::cout << LOG_FIX << "Init Server Backend Result: " << InitSteamworksServer(This->SteamSubsystem) << std::endl;
            //std::cout << bSteamworksGameServerInitialized << std::endl;
            *(&This->SteamSubsystem->bTickerStarted + 9) = true;
            IsInit = true;
        }
#endif

        bool ret = CreateSessionHook.CallOriginal(This, HostingPlayerNum, SessionName, NewSessionSettings);
        std::cout<< LOG_FIX << "Started Registration: " << ret << std::endl;
        Session = This;
        return ret;
    });

static bool DisplayStartingConnectionMessage = true;
Hook<void(FOnlineAsyncTaskSteamCreateServer*)> FOnlineAsyncTaskSteamCreateServer_TickHook("40 55 56 48 83 EC 48 48",
    [](FOnlineAsyncTaskSteamCreateServer* This) -> void
    {
        FOnlineAsyncTaskSteamCreateServer_TickHook.CallOriginal(This);

        if (DisplayStartingConnectionMessage)
        {
            std::cout<< LOG_FIX << "Attempting to connect to Steam dedicated servers..." << std::endl;
            DisplayStartingConnectionMessage = false;
        }
        /*
        std::cout << "Init?: " << This->bInit << "\nComplete?: " << This->bIsComplete.Get() << "\nSuccessful?: " << This->bWasSuccessful.Get() << std::endl;
        if (Session)
        {
            if (Session->GameServerSteamId.Object)
            {
                std::cout << "GameServerID: " << Session->GameServerSteamId.Object->UniqueNetId << std::endl;
            }
            std::cout << "Policy?: " << Session->bPolicyResponseReceived << "\nServer Connection?: " << Session->bSteamworksGameServerConnected << std::endl;
        }
        */
        if (This->bInit && This->bIsComplete.Get() && This->bWasSuccessful.Get())
        {
            std::cout<< LOG_FIX << "Connected and registered with Steam dedicated servers!\n";
            DisplayStartingConnectionMessage = true;
        } else if (!This->bInit && This->bIsComplete.Get() && !This->bWasSuccessful.Get())
        {
            std::cout<< LOG_FIX << "Failed to connect to Steam dedicated servers..." << std::endl;
            DisplayStartingConnectionMessage = true;
        }
    });

void SetupSessionSpooferHooks()
{
    CreateSessionHook.Create();
    CreateSessionHook.Enable();

    FOnlineAsyncTaskSteamCreateServer_TickHook.Create();
    FOnlineAsyncTaskSteamCreateServer_TickHook.Enable();

}