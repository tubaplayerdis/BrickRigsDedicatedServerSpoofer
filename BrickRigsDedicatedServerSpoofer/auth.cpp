#include "auth.hpp"
#include <json.hpp>
#include <BR-SDK.hpp>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

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

    SDK::ABrickPlayerController* PlayerController = reinterpret_cast<SDK::ABrickPlayerController*>(SDK::UGameplayStatics::GetPlayerController(SDK::UWorld::GetWorld(), 0));
    SDK::FString PlayerNetID = SDK::UBrickStatics::UniqueNetIdToString(PlayerController->GetPlayerId());
    bool auth = false;
    for (auto User : RegisteredUsers)
    {
        if (User == PlayerNetID.ToString()) auth = true;
    }
    if (!auth)
    {
        MessageBoxA(NULL, "Authentication Failed. This software will now uninject", SOFTWARE_NAME, MB_OK);
        return false;
    }
    std::cout << "AUTHENTICATED USER: " << PlayerNetID.ToString() << std::endl;

    return true;
}
