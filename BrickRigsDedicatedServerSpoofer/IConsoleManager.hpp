#pragma once
#include <BR-SDK.hpp>

#include "SteamSubsystem.hpp"

FARPROC GetImportedFunctionAddress(HMODULE Module, const char* ImportModuleName, const char* FuncName);



struct IConsoleManager_vtbl;
struct IConsoleObject;
struct IConsoleThreadPropagation;
struct IConsoleManager
{
    IConsoleManager_vtbl *__vftable /*VFT*/;
};

struct FConsoleManager : IConsoleManager
{
    UC::TMap<UC::FString,IConsoleObject *> ConsoleObjects;
    bool bHistoryWasLoaded;
    UC::TMap<UC::FString,UC::TArray<UC::FString>> HistoryEntriesMap;
    UC::TArray<SDK::TDelegate<void __cdecl(void)>> ConsoleVariableChangeSinks;
    IConsoleThreadPropagation *ThreadPropagationCallback;
    bool bCallAllConsoleVariableSinks;
    FWindowsCriticalSection ConsoleObjectsSynchronizationObject;

    //This only prints CVars.
    static void PrintCommands();
};
static_assert(sizeof(FConsoleManager) == 0xF8);