#include "IConsoleManager.hpp"
#include <BR-SDK.hpp>
#include <thread>

#include "MACROS.hpp"

std::uintptr_t O_ConsoleManagerSingleton = 0;
void OConsoleManagerSingleton()
{
    if (O_ConsoleManagerSingleton == 0)
    {
        Function<void()> IConsoleManager_SetupSingletonSig("48 83 EC 28 48 83 3D ?? ?? ?? ?? ?? 0F 85 EC");
        IConsoleManager_SetupSingletonSig();
        // ^ adjust to a signature that's unique/stable in your binary; the first
        // few bytes of SetupSingleton (sub rsp, 28h; cmp cs:Singleton...) work here,
        // but verify uniqueness in IDA like you did for the others.

        uintptr_t FuncAddr = IConsoleManager_SetupSingletonSig.GetPtr();

        if (FuncAddr == 0)
        {
            std::cerr << "ConsoleManager singleton NOT FOUND (SetupSingleton sig missed)\n";
            return;
        }

        uint8_t* Bytes = reinterpret_cast<uint8_t*>(FuncAddr);

        constexpr int ScanRange = 0x110; // mov ref sits at +0xE9, give headroom

        for (int i = 0; i < ScanRange - 7; i++)
        {
            // mov [rip+disp32], rdi -> 48 89 3D
            bool IsMovSingleton = Bytes[i] == 0x48 && Bytes[i + 1] == 0x89 && Bytes[i + 2] == 0x3D;

            if (!IsMovSingleton)
                continue;

            int32_t RelOffset = *reinterpret_cast<int32_t*>(&Bytes[i + 3]);
            uintptr_t InstrEnd = FuncAddr + i + 7;
            uintptr_t SingletonSlotAddr = InstrEnd + RelOffset; // address of the static pointer slot

            O_ConsoleManagerSingleton = SingletonSlotAddr;
            break;
        }
    }

    if (O_ConsoleManagerSingleton == 0)
        std::cerr << "ConsoleManager singleton offset NOT FOUND\n";
}

void FConsoleManager::PrintCommands()
{
    OConsoleManagerSingleton();
    FConsoleManager* Manager = *reinterpret_cast<FConsoleManager**>(O_ConsoleManagerSingleton);
    for (SDK::TPair<SDK::FString, IConsoleObject*> Object : Manager->ConsoleObjects)
    {
        std::cout << Object.First.ToString() << std::endl;
    }
}

/* 11 */
enum ELogVerbosity : __int8
{
    NoLogging = 0x0,
    Fatal = 0x1,
    Error = 0x2,
    Warning = 0x3,
    Display = 0x4,
    Log = 0x5,
    Verbose = 0x6,
    VeryVerbose = 0x7,
    All = 0x7,
    NumVerbosity = 0x8,
    VerbosityMask = 0xF,
    SetColor = 0x40,
    BreakOnLog = 0x80,
  };

class FOutputDevice
{
public:
    virtual ~FOutputDevice() {}
    virtual void Serialize(const wchar_t* V, ELogVerbosity Verbosity, const SDK::FName& Category, const long double Time) = 0;
    virtual void Serialize(const wchar_t* V, ELogVerbosity Verbosity, const SDK::FName& Category) = 0;
    virtual void Flush() {}
    virtual void TearDown() {}
    virtual void Dump(void* Archive) {}
    virtual bool IsMemoryOnly() { return false; }
    virtual bool CanBeUsedOnAnyThread() const { return false; }
    virtual bool CanBeUsedOnMultipleThreads() const { return false; }

    bool bSuppressEventTag = false;
    bool bAutoEmitLineTerminator = true;
};

class MyConsoleOutputDevice : public FOutputDevice
{
public:
    void Serialize(const wchar_t* V, ELogVerbosity Verbosity, const SDK::FName& Category, const long double Time) override
    {
        Print(V);
    }

    void Serialize(const wchar_t* V, ELogVerbosity Verbosity, const SDK::FName& Category) override
    {
        Print(V);
    }

    bool CanBeUsedOnAnyThread() const override { return true; }
    bool CanBeUsedOnMultipleThreads() const override { return true; }

private:
    void Print(const wchar_t* V)
    {
        std::wcout << L"[CMD LOG] " << (V ? V : L"(null)") << L"\n";
    }
};
static MyConsoleOutputDevice MyDevice;


typedef void FOutputDeviceRedirector;

Function<FOutputDeviceRedirector*()> GetGlobalLogSingleton("48 83 EC 28 8B 0D ?? ?? ?? ?? 65 48 8B 04 25 58 00 00 00 BA 08 01 00 00 48 8B 04 C8 8B 04 02 39 05 ?? ?? ?? ?? 7F 0C 48 8D 05 ?? ?? ?? ?? 48 83 C4 28 C3 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 3D ?? ?? ?? ?? ?? 75 DF 48 8D 05 ?? ?? ?? ?? 48 89 5C 24 20 33 DB 66");
Function<void(FOutputDeviceRedirector*, FOutputDevice*)> AddOutputDevice("48 85 D2 0F 84 A9 00 00 00 48 89 54");

void SetupConsoleOutputOverride()
{
    void* LogSingleton = GetGlobalLogSingleton();
    AddOutputDevice(LogSingleton, &MyDevice);

    std::thread([]() {
        int i = 10;
        while (i > 0)
        {
            Sleep(1000);
            auto Engine = SDK::UEngine::GetEngine();
            auto World = SDK::UWorld::GetWorld();
            if (Engine && World)
            {
                SDK::UInputSettings::GetDefaultObj()->ConsoleKeys[0].KeyName = SDK::UKismetStringLibrary::Conv_StringToName(L"F2");
                SDK::UObject* NewObject = SDK::UGameplayStatics::SpawnObject(Engine->ConsoleClass, Engine->GameViewport);
                Engine->GameViewport->ViewportConsole = static_cast<SDK::UConsole*>(NewObject);
                std::cout<< LOG_FIX << "Press F2 to use the console" << std::endl;
                return;
            }
            i--;
        }

    }).detach();
}
