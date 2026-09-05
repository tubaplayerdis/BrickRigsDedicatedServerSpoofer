#include "auth.hpp"
#include <json.hpp>
#include <BR-SDK.hpp>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

// Function pointer typedefs matching Steam's exported flat API
typedef bool (__cdecl* SteamAPI_Init_t)();
typedef int  (__cdecl* SteamAPI_GetHSteamUser_t)();
typedef void* (__cdecl* SteamInternal_FindOrCreateUserInterface_t)(int hSteamUser, const char* version);
typedef uint64_t (__cdecl* SteamAPI_ISteamUser_GetSteamID_t)(void* instancePtr);

uint64_t GetSteamID()
{
    HMODULE hSteam = GetModuleHandleA("steam_api64.dll");
    if (!hSteam)
    {
        printf("steam_api64.dll not loaded in this process\n");
        return 1;
    }

    auto SteamAPI_Init = (SteamAPI_Init_t)GetProcAddress(hSteam, "SteamAPI_Init");
    auto SteamAPI_GetHSteamUser = (SteamAPI_GetHSteamUser_t)GetProcAddress(hSteam, "SteamAPI_GetHSteamUser");
    auto SteamInternal_FindOrCreateUserInterface = (SteamInternal_FindOrCreateUserInterface_t)GetProcAddress(hSteam, "SteamInternal_FindOrCreateUserInterface");
    auto SteamAPI_ISteamUser_GetSteamID = (SteamAPI_ISteamUser_GetSteamID_t)GetProcAddress(hSteam, "SteamAPI_ISteamUser_GetSteamID");

    if (!SteamAPI_GetHSteamUser || !SteamInternal_FindOrCreateUserInterface || !SteamAPI_ISteamUser_GetSteamID)
    {
        printf("Missing one or more expected exports\n");
        std::cout
        << SteamAPI_GetHSteamUser << "\n"
        << SteamInternal_FindOrCreateUserInterface << "\n"
        << SteamAPI_ISteamUser_GetSteamID << "\n";
        return 0;
    }

    int hUser = SteamAPI_GetHSteamUser();

    // Version string must match an interface version the installed steam_api.dll actually supports.
    void* pUser = SteamInternal_FindOrCreateUserInterface(hUser, "SteamUser023");
    if (!pUser) { printf("Failed to get ISteamUser interface\n"); return 0; }

    uint64_t steamID = SteamAPI_ISteamUser_GetSteamID(pUser);
    return steamID;

    return 0;
}

std::string HttpsGet(const std::wstring& host, const std::wstring& path, bool& success) {
    std::string result;
    success = false;

    HINTERNET hSession = WinHttpOpen(L"BRDS-Client/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return result;

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return result; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return result; }

    if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
        WinHttpReceiveResponse(hRequest, NULL))
    {
        DWORD statusCode = 0, statusSize = sizeof(statusCode);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_FLAG_NUMBER | WINHTTP_QUERY_STATUS_CODE,
            WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusSize, WINHTTP_NO_HEADER_INDEX);

        if (statusCode == 200) {
            DWORD size = 0;
            do {
                DWORD downloaded = 0;
                WinHttpQueryDataAvailable(hRequest, &size);
                if (size == 0) break;
                std::vector<char> buf(size);
                WinHttpReadData(hRequest, buf.data(), size, &downloaded);
                result.append(buf.data(), downloaded);
            } while (size > 0);
            success = true;
        } else {
            std::string message = "Failed to auth BRDS: Bad Status Code: " + std::to_string(statusCode);
            MessageBoxA(NULL, message.c_str(), SOFTWARE_NAME, MB_OK);
        }
    } else {
        MessageBoxA(NULL, "Failed to auth BRDS: Network Error", SOFTWARE_NAME, MB_OK);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return result;
}

std::vector<std::string> RegisteredUsers = { /*"76561199077491485"*/ };

bool auth::GetAuthed()
{
    using json = nlohmann::json;

    bool ok = false;
    std::string body = HttpsGet(L"aaronwilk.dev", L"/brds", ok);

    if (ok) {
        try {
            json data = json::parse(body);
            RegisteredUsers = data.get<std::vector<std::string>>();
        }
        catch (const json::parse_error& e) {
            std::cout << "parse error\n";
            return 1;
        }
    }

    auto SteamID = GetSteamID();
    bool auth = false;
    for (auto User : RegisteredUsers)
    {
        if (User == std::to_string(SteamID)) auth = true;
    }
    if (!auth)
    {
        MessageBoxA(NULL, "Authentication Failed. This software will now uninject", SOFTWARE_NAME, MB_OK);
        return false;
    }
    std::cout << "AUTHENTICATED USER: " << SteamID << std::endl;

    return true;
}
