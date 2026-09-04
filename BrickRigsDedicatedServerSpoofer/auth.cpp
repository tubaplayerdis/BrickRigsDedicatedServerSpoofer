#include "auth.hpp"
#include <json.hpp>
#include <httplib.h>
#include <BR-SDK.hpp>

std::vector<std::string> RegisteredUsers = { /*"76561199077491485"*/ };

bool auth::GetAuthed()
{
    return true;
    using json = nlohmann::json;

    //TODO: Crashes
    try
    {
        httplib::Client cli("https://aaronwilk.dev");

        if (auto res = cli.Get("/brds"))
        {
            if (res->status == 200) {
                try {
                    json data = json::parse(res->body);
                    std::vector<std::string> valid_ids = data.get<std::vector<std::string>>();
                    RegisteredUsers = valid_ids;
                }
                catch (const json::parse_error& e)
                {
                    std::cout << "parse error\n";
                    return 1;
                }

            }
            else {
                MessageBoxA(NULL, "Failed to auth BRDS: Bad Status Code", "BRDS", MB_OK);
            }
        }
        else
        {
            MessageBoxA(NULL, "Failed to auth BRDS: Network Error", "BRDS", MB_OK);
        }
    }
    catch (std::exception &e)
    {
        std::string message = "Failed to auth BRDS: ";
        message += e.what();
        MessageBoxA(NULL, message.c_str(), "BRDS", MB_OK);
    }
    httplib::Client cli("https://aaronwilk.dev");

    SDK::ABrickPlayerController* PlayerController = reinterpret_cast<SDK::ABrickPlayerController*>(SDK::UGameplayStatics::GetPlayerController(SDK::UWorld::GetWorld(), 0));
    SDK::FString PlayerNetID = SDK::UBrickStatics::UniqueNetIdToString(PlayerController->GetPlayerId());
    bool auth = false;
    for (auto User : RegisteredUsers)
    {
        if (User == PlayerNetID.ToString()) auth = true;
    }
    if (!auth)
    {
        MessageBoxA(NULL, "Authentication Failed. This software will now uninject", "BRDS", MB_OK);
        return false;
    }

    return true;
}
