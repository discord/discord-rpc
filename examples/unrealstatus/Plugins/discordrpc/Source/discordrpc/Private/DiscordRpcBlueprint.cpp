

#include "DiscordRpcBlueprint.h"

#include "discord-rpc.h"

DEFINE_LOG_CATEGORY(Discord)

static UDiscordRpc* self = nullptr;
static void ReadyHandler() {
    UE_LOG(Discord, Log, TEXT("Discord connected"));
    if (self) {
        self->IsConnected = true;
    }
}

static void DisconnectHandler(int errorCode, const char* message) {
    UE_LOG(Discord, Log, TEXT("Discord disconnected (%d): %s"), errorCode, message);
    if (self) {
        self->IsConnected = false;
    }
};

void UDiscordRpc::Initialize(const FString& applicationId, bool autoRegister)
{
    self = this;
    IsConnected = false;
    DiscordEventHandlers handlers{};
    handlers.ready = ReadyHandler;
    handlers.disconnected = DisconnectHandler;
    auto appId = StringCast<ANSICHAR>(*applicationId);
    Discord_Initialize((const char*)appId.Get(), &handlers, autoRegister);
}

void UDiscordRpc::Shutdown()
{
    Discord_Shutdown();
    self = nullptr;
}

void UDiscordRpc::RunCallbacks()
{
    Discord_RunCallbacks();
}
