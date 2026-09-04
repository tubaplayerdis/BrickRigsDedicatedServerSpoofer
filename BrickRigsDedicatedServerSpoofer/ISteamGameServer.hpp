#pragma once
#include <BR-SDK.hpp>

struct CSteamID
{
	/* 151016 */
	union SteamID_t
	{
		struct SteamIDComponent_t
		{
			unsigned __int32 m_unAccountID : 32;
			unsigned __int32 m_unAccountInstance : 20;
			unsigned __int32 m_EAccountType : 4;
			__int32 m_EUniverse : 8;
		};

		SteamIDComponent_t m_comp;
		unsigned __int64 m_unAll64Bits;
	};
	SteamID_t m_steamid;
};
struct SteamNetworkingIdentity;
struct ISteamGameServer;
struct SteamIPAddress_t;

/* 3229 */
enum EBeginAuthSessionResult : __int32
{
	k_EBeginAuthSessionResultOK = 0x0,
	k_EBeginAuthSessionResultInvalidTicket = 0x1,
	k_EBeginAuthSessionResultDuplicateRequest = 0x2,
	k_EBeginAuthSessionResultInvalidVersion = 0x3,
	k_EBeginAuthSessionResultGameMismatch = 0x4,
	k_EBeginAuthSessionResultExpiredTicket = 0x5,
};

/* 3514 */
enum EUserHasLicenseForAppResult : __int32
{
	k_EUserHasLicenseResultHasLicense = 0x0,
	k_EUserHasLicenseResultDoesNotHaveLicense = 0x1,
	k_EUserHasLicenseResultNoAuth = 0x2,
};

/* 151110 */
struct /*VFT*/ ISteamGameServer_vtbl
{
	bool(__fastcall* InitGameServer)(ISteamGameServer* This, unsigned int, unsigned __int16, unsigned __int16, unsigned int, unsigned int, const char*);
	void(__fastcall* SetProduct)(ISteamGameServer* This, const char*);
	void(__fastcall* SetGameDescription)(ISteamGameServer* This, const char*);
	void(__fastcall* SetModDir)(ISteamGameServer* This, const char*);
	void(__fastcall* SetDedicatedServer)(ISteamGameServer* This, bool);
	void(__fastcall* LogOn)(ISteamGameServer* This, const char*);
	void(__fastcall* LogOnAnonymous)(ISteamGameServer* This);
	void(__fastcall* LogOff)(ISteamGameServer* This);
	bool(__fastcall* BLoggedOn)(ISteamGameServer* This);
	bool(__fastcall* BSecure)(ISteamGameServer* This);
	CSteamID* (__fastcall* GetSteamID)(ISteamGameServer* This, CSteamID* result);
	bool(__fastcall* WasRestartRequested)(ISteamGameServer* This);
	void(__fastcall* SetMaxPlayerCount)(ISteamGameServer* This, int);
	void(__fastcall* SetBotPlayerCount)(ISteamGameServer* This, int);
	void(__fastcall* SetServerName)(ISteamGameServer* This, const char*);
	void(__fastcall* SetMapName)(ISteamGameServer* This, const char*);
	void(__fastcall* SetPasswordProtected)(ISteamGameServer* This, bool);
	void(__fastcall* SetSpectatorPort)(ISteamGameServer* This, unsigned __int16);
	void(__fastcall* SetSpectatorServerName)(ISteamGameServer* This, const char*);
	void(__fastcall* ClearAllKeyValues)(ISteamGameServer* This);
	void(__fastcall* SetKeyValue)(ISteamGameServer* This, const char*, const char*);
	void(__fastcall* SetGameTags)(ISteamGameServer* This, const char*);
	void(__fastcall* SetGameData)(ISteamGameServer* This, const char*);
	void(__fastcall* SetRegion)(ISteamGameServer* This, const char*);
	void(__fastcall* SetAdvertiseServerActive)(ISteamGameServer* This, bool);
	unsigned int(__fastcall* GetAuthSessionTicket)(ISteamGameServer* This, void*, int, unsigned int*, const SteamNetworkingIdentity*);
	EBeginAuthSessionResult(__fastcall* BeginAuthSession)(ISteamGameServer* This, const void*, int, CSteamID);
	void(__fastcall* EndAuthSession)(ISteamGameServer* This, CSteamID);
	void(__fastcall* CancelAuthTicket)(ISteamGameServer* This, unsigned int);
	EUserHasLicenseForAppResult(__fastcall* UserHasLicenseForApp)(ISteamGameServer* This, CSteamID, unsigned int);
	bool(__fastcall* RequestUserGroupStatus)(ISteamGameServer* This, CSteamID, CSteamID);
	void(__fastcall* GetGameplayStats)(ISteamGameServer* This);
	unsigned __int64(__fastcall* GetServerReputation)(ISteamGameServer* This);
	SteamIPAddress_t* (__fastcall* GetPublicIP)(ISteamGameServer* This, SteamIPAddress_t* result);
	bool(__fastcall* HandleIncomingPacket)(ISteamGameServer* This, const void*, int, unsigned int, unsigned __int16);
	int(__fastcall* GetNextOutgoingPacket)(ISteamGameServer* This, void*, int, unsigned int*, unsigned __int16*);
	unsigned __int64(__fastcall* AssociateWithClan)(ISteamGameServer* This, CSteamID);
	unsigned __int64(__fastcall* ComputeNewPlayerCompatibility)(ISteamGameServer* This, CSteamID);
	bool(__fastcall* SendUserConnectAndAuthenticate_DEPRECATED)(ISteamGameServer* This, unsigned int, const void*, unsigned int, CSteamID*);
	CSteamID* (__fastcall* CreateUnauthenticatedUserConnection)(ISteamGameServer* This, CSteamID* result);
	void(__fastcall* SendUserDisconnect_DEPRECATED)(ISteamGameServer* This, CSteamID);
	bool(__fastcall* BUpdateUserData)(ISteamGameServer* This, CSteamID, const char*, unsigned int);
	void(__fastcall* SetMasterServerHeartbeatInterval_DEPRECATED)(ISteamGameServer* This, int);
	void(__fastcall* ForceMasterServerHeartbeat_DEPRECATED)(ISteamGameServer* This);
};

/* 151107 */
struct ISteamGameServer
{
	ISteamGameServer_vtbl* __vftable /*VFT*/;
};

struct FOnlineAsyncItem_vtbl;

/* 151121 */
struct FOnlineAsyncItem
{
	FOnlineAsyncItem_vtbl* __vftable /*VFT*/;
	long double StartTime;
};

/* 151123 */
struct FOnlineAsyncTask : FOnlineAsyncItem
{
};

/* 144065 */
struct FThreadSafeCounter
{
	volatile int Counter;
};

/* 144752 */
struct FThreadSafeBool : FThreadSafeCounter
{
	void Set(bool value)
	{
		Counter = (int)value;
	}

	bool Get()
	{
		return Counter == 1;
	}
};

/* 155528 */
struct FOnlineAsyncTaskBasic : FOnlineAsyncTask
{
	FOnlineSubsystemSteam* Subsystem;
	FThreadSafeBool bIsComplete;
	FThreadSafeBool bWasSuccessful;
};

/* 155530 */
struct FOnlineAsyncTaskSteam : FOnlineAsyncTaskBasic
{
	unsigned __int64 CallbackHandle;
};

struct __declspec(align(8)) FOnlineAsyncTaskSteamCreateServer : FOnlineAsyncTaskSteam
{
	bool bInit;
	SDK::FName SessionName;
};

