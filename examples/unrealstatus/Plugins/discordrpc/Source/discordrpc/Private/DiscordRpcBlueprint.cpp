

#include "DiscordRpcBlueprint.h"

#include "discord-rpc.h"

DEFINE_LOG_CATEGORY(DiscordLogCategory)

/*static*/ void UDiscordRpcBlueprint::Initialize(const FString& applicationId, bool autoRegister)
{
    DiscordEventHandlers handlers{};
    handlers.ready = []() {
        UE_LOG(DiscordLogCategory, Log, TEXT("Discord connected"));
    };
    handlers.disconnected = [](int errorCode, const char* message) {
        UE_LOG(DiscordLogCategory, Log, TEXT("Discord disconnected (%d): %s"), errorCode, message);
    };
    auto appId = StringCast<ANSICHAR>(*applicationId);
    Discord_Initialize((const char*)appId.Get(), &handlers, autoRegister);
}

/*static*/ void UDiscordRpcBlueprint::Shutdown()
{
    Discord_Shutdown();
}

/*static*/ void UDiscordRpcBlueprint::RunCallbacks()
{
    Discord_RunCallbacks();
}
