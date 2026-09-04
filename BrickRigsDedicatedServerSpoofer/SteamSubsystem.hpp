#pragma once
#include <BR-SDK.hpp>


struct FScriptContainerElement;
struct FRunnableThread;
struct IOnlineVoice;
struct IOnlineSubsystem_vtbl;
struct FOnlineNotificationHandler;
struct FOnlineNotificationTransportManager;
struct FTickerObjectBase_vtbl;
struct FTicker;
struct UNamedInterfaces;
struct FOnlineSessionSteam;
struct FOnlineIdentitySteam;
struct FOnlineFriendsSteam;
struct FOnlineUserSteam;
struct FOnlineSharedCloudSteam;
struct FOnlineUserCloudSteam;
struct FOnlineLeaderboardsSteam;
struct FOnlineExternalUISteam;
struct FOnlineAchievementsSteam;
struct FOnlinePresenceSteam;
struct FOnlineAuthSteam;
struct FOnlineAuthUtilsSteam;
struct FOnlinePingInterfaceSteam;
struct FOnlineEncryptedAppTicketSteam;
struct FOnlineAsyncTaskManagerSteam;
struct FSteamClientInstanceHandler;
struct FSteamServerInstanceHandler;
struct IOnlineSession_vtbl;
struct FLANSession;

const struct __declspec(align(8)) FOnlineSessionSearchResult
{
	unsigned __int8 padding[0x118];
};

template<typename T, int>
struct TSharedPtr
{
	T* Object;
	UC::int8 SharedReferenceCount[0x8];
};

template<typename T>
struct TWeakPtr
{
	T* Object;
	UC::int8 WeakReferenceCount[0x8];
};

struct SharedFromThisFUnqiueNetId
{
	UC::int8 WeakThis[0x10];
};

struct FUniqueNetId_vtbl;

const struct FUniqueNetId : SharedFromThisFUnqiueNetId
{
	FUniqueNetId_vtbl* __vftable /*VFT*/;
};

template<typename>
struct TMulticastDelegate
{
	UC::int8 pad[0x18];
};

/* 150426 */
struct IOnlineSubsystem
{
	IOnlineSubsystem_vtbl* __vftable /*VFT*/;
	TSharedPtr<FOnlineNotificationHandler, 1> OnlineNotificationHandler;
	TSharedPtr<FOnlineNotificationTransportManager, 1> OnlineNotificationTransportManager;
	TMulticastDelegate<void> OnConnectionStatusChangedDelegates;
	TMulticastDelegate<void> OnOnlineEnvironmentChangedDelegates;
};

/* 144768 */
struct FDelegateHandle
{
	unsigned __int64 ID;
};

/* 150976 */
struct FTickerObjectBase
{
	FTickerObjectBase_vtbl* __vftable /*VFT*/;
	FTicker* Ticker;
	FDelegateHandle TickHandle;
};

/* 144103 */
struct FNameEntryId
{
	unsigned int Value;
};

template<typename>
struct TQueue
{
	UC::int8 pad[0x10];
};

template<typename T>
struct TSharedRef
{
	T* Object;
	UC::int8 SharedReferenceCount[0x8];
};

/* 150992 */
struct __declspec(align(16)) FOnlineSubsystemImpl : IOnlineSubsystem, FTickerObjectBase
{
	SDK::FName SubsystemName;
	SDK::FName InstanceName;
	bool bForceDedicated;
	UNamedInterfaces* NamedInterfaces;
	TQueue<SDK::TDelegate<void __cdecl(void)>> NextTickQueue;
	SDK::TArray<SDK::TDelegate<void __cdecl(void)>> CurrentTickBuffer;
	bool bTickerStarted;
};

/* 144885 */
struct FWindowsCriticalSection
{
	CRITICAL_SECTION CriticalSection;
};

/* 150995 */
struct FOnlineSubsystemSteam : FOnlineSubsystemImpl
{
	int GameServerGamePort;
	int GameServerQueryPort;
	SDK::TArray<void*> UserCloudData;
	TSharedPtr<FOnlineSessionSteam, 1> SessionInterface;
	TSharedPtr<FOnlineIdentitySteam, 1> IdentityInterface;
	TSharedPtr<FOnlineFriendsSteam, 1> FriendInterface;
	TSharedPtr<FOnlineUserSteam, 1> UserInterface;
	TSharedPtr<FOnlineSharedCloudSteam, 1> SharedCloudInterface;
	TSharedPtr<FOnlineUserCloudSteam, 1> UserCloudInterface;
	TSharedPtr<FOnlineLeaderboardsSteam, 1> LeaderboardsInterface;
	TSharedPtr<IOnlineVoice, 1> VoiceInterface;
	bool bVoiceInterfaceInitialized;
	TSharedPtr<FOnlineExternalUISteam, 1> ExternalUIInterface;
	TSharedPtr<FOnlineAchievementsSteam, 1> AchievementsInterface;
	TSharedPtr<FOnlinePresenceSteam, 1> PresenceInterface;
	TSharedPtr<FOnlineAuthSteam, 1> AuthInterface;
	TSharedPtr<FOnlineAuthUtilsSteam, 1> AuthInterfaceUtils;
	TSharedPtr<FOnlinePingInterfaceSteam, 1> PingInterface;
	TSharedPtr<FOnlineEncryptedAppTicketSteam, 1> EncryptedAppTicketInterface;
	FOnlineAsyncTaskManagerSteam* OnlineAsyncTaskThreadRunnable;
	FRunnableThread* OnlineAsyncTaskThread;
	TSharedPtr<FSteamClientInstanceHandler, 0> SteamAPIClientHandle;
	TSharedPtr<FSteamServerInstanceHandler, 0> SteamAPIServerHandle;
	FWindowsCriticalSection UserCloudDataLock;
	TMulticastDelegate<void __cdecl(bool)> OnSteamServerLoginCompletedDelegates;
};

/* 148819 */
struct FOnlineSessionSettings
{
	void* __vftable /*VFT*/;
	SDK::FString SessionName;
	int NumPublicConnections;
	int NumPrivateConnections;
	bool bShouldAdvertise;
	bool bAllowJoinInProgress;
	bool bIsLANMatch;
	bool bIsDedicated;
	bool bUsesStats;
	bool bAllowInvites;
	bool bUsesPresence;
	bool bAllowJoinViaPresence;
	bool bAllowJoinViaPresenceFriendsOnly;
	bool bAntiCheatProtected;
	bool bUseLobbiesIfAvailable;
	bool bUseLobbiesVoiceChatIfAvailable;
	int BuildUniqueId;
	UC::uint8 pad0[0x50]; //Settings;
	UC::uint8 pad_1[0x50];//memberSettings
};
static_assert(sizeof(FOnlineSessionSettings) == 0xD0);

/* 89 */
enum EShaderPrecisionModifier_Type : __int32
{
	Float = 0x0,
	Half = 0x1,
	Fixed = 0x2,
};

/* 2487 */
typedef EShaderPrecisionModifier_Type EOnlineKeyValuePairDataType_Type;


/* 150544 */
union FVariantData_ValueUnion
{
	bool AsBool;
	int AsInt;
	unsigned int AsUInt;
	float AsFloat;
	__int64 AsInt64;
	unsigned __int64 AsUInt64;
	long double AsDouble;
	wchar_t* AsTCHAR;
};

/* 150545 */
struct FVariantData
{
	EOnlineKeyValuePairDataType_Type Type;
	FVariantData_ValueUnion Value;
};

/* 1234 */
enum EOnlineComparisonOp_Type : __int32
{
	Equals = 0x0,
	NotEquals = 0x1,
	GreaterThan = 0x2,
	GreaterThanEquals = 0x3,
	LessThan = 0x4,
	LessThanEquals = 0x5,
	Near = 0x6,
	In = 0x7,
	NotIn = 0x8,
};

/* 155736 */
struct FOnlineSessionSearchParam
{
	FVariantData Data;
	EOnlineComparisonOp_Type ComparisonOp;
	int ID;
};



struct IOnlineSession
{
	IOnlineSession_vtbl* __vftable /*VFT*/;
	UC::int8 Delegates[0x278 - sizeof(void*)];
};

struct FOnlineSession_vtbl;
struct FOnlineSessionInfo;

/* 148817 */
struct FOnlineSession
{
	FOnlineSession_vtbl* __vftable /*VFT*/;
	TSharedPtr<FUniqueNetId, 0> OwningUserId;
	SDK::FString OwningUserName;
	FOnlineSessionSettings SessionSettings;
	TSharedPtr<FOnlineSessionInfo, 0> SessionInfo;
	int NumOpenPrivateConnections;
	int NumOpenPublicConnections;
};

/* 1204 */
enum EOnlineAsyncTaskState : __int32
{
	NotStarted = 0x0,
	InProgress = 0x1,
	Done = 0x2,
	Failed = 0x3,
};

/* 1224 */
typedef EOnlineAsyncTaskState EOnlineSessionState;

/* 150427 */
struct __declspec(align(8)) FNamedOnlineSession : FOnlineSession
{
	const SDK::FName SessionName;
	int HostingPlayerNum;
	bool bHosting;
	TSharedPtr<FUniqueNetId const, 0> LocalOwnerId;
	SDK::TArray<TSharedRef<FUniqueNetId const>> RegisteredPlayers;
	EOnlineSessionState SessionState;
};

/* 151004 */
const struct FUniqueNetIdSteam : FUniqueNetId
{
	unsigned __int64 UniqueNetId;
};

struct FOnlineSessionSearch_vtbl;
struct FOnlineSearchSettings_vtbl;
struct FOnlineSessionSearchParam;
struct FOnlineSessionSearchResult;

enum ENetworkReplayError_Type : __int32
{
	None = 0x0,
	ServiceUnavailable = 0x1,
};

/* 1398 */
typedef ENetworkReplayError_Type ESteamSession_Type;


struct FPendingInviteData
{
	ESteamSession_Type PendingInviteType;
	TSharedRef<FUniqueNetIdSteam const> LobbyId;
	SDK::FString ServerIp;
};


struct FOnlineSearchSettings
{
	FOnlineSearchSettings_vtbl* __vftable /*VFT*/;
	SDK::TMap <SDK::FName, FOnlineSessionSearchParam> SearchParams;
};

struct FOnlineSessionSearch
{
	FOnlineSessionSearch_vtbl* __vftable /*VFT*/;
	SDK::TArray<FOnlineSessionSearchResult> SearchResults;
	EOnlineAsyncTaskState SearchState;
	int MaxSearchResults;
	FOnlineSearchSettings QuerySettings;
	bool bIsLanQuery;
	int PingBucketSize;
	int PlatformHash;
	float TimeoutInSeconds;
};

struct FOnlineSessionSteam : IOnlineSession
{
	FOnlineSubsystemSteam* SteamSubsystem;
	FLANSession* LANSession;
	FWindowsCriticalSection SessionLock;
	SDK::TArray<FNamedOnlineSession> Sessions;
	bool bSteamworksGameServerConnected;
	TSharedPtr<FUniqueNetIdSteam const, 0> GameServerSteamId;
	bool bPolicyResponseReceived;
	TSharedPtr<FOnlineSessionSearch, 0> CurrentSessionSearch;
	FPendingInviteData PendingInvite;
	SDK::TArray<TSharedRef<FUniqueNetIdSteam const>> PendingSearchLobbyIds;
	FWindowsCriticalSection JoinedLobbyLock;
	SDK::TArray<TSharedRef<FUniqueNetIdSteam const>> JoinedLobbyList;
};

