
#include <vector>
#include <functional>
#include <mutex>
#include <Hooking/Hook.hpp>
#include <Hooking/Signature.hpp>

std::mutex main_mutex;
auto MainThreadFunctions = std::vector<std::function<void()>>();

void HookedTick(void* EngineLoopPtr);
Hook<void(void*)> EngineLoopHook("48 8B C4 48 89 58 ?? 48 89 70 ?? 48 89 78 ?? 55 41 54 41 55 41 56 41 57 48 8D 68 ?? 48 81 EC ?? ?? ?? ?? 0F 29 70 ?? 48 8D 15 ?? ?? ?? ?? 48 8D 44 24 ??", HookedTick);

void HookedTick(void* EngineLoopPtr)
{
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

void SetupMainThreadExecution()
{
    EngineLoopHook.Create();
    EngineLoopHook.Enable();
}

void RunOnMainThread(std::function<void()> func)
{
    std::unique_lock lock(main_mutex);
    MainThreadFunctions.push_back(func);
}
