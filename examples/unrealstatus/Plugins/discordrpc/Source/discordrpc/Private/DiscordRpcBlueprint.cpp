

#include "DiscordRpcBlueprint.h"

#include "discord-rpc.h"

DEFINE_LOG_CATEGORY(Discord)

static UDiscordRpc* self = nullptr;
static void ReadyHandler()
{
    UE_LOG(Discord, Log, TEXT("Discord connected"));
    if (self) {
        self->IsConnected = true;
        self->OnConnected.Broadcast();
    }
}

static void DisconnectHandler(int errorCode, const char* message)
{
    auto msg = FString(message);
    UE_LOG(Discord, Log, TEXT("Discord disconnected (%d): %s"), errorCode, *msg);
    if (self) {
        self->IsConnected = false;
        self->OnDisconnected.Broadcast(errorCode, msg);
    }
}

static void ErroredHandler(int errorCode, const char* message)
{
    auto msg = FString(message);
    UE_LOG(Discord, Log, TEXT("Discord error (%d): %s"), errorCode, *msg);
    if (self) {
        self->OnErrored.Broadcast(errorCode, msg);
    }
}

static void JoinGameHandler(const char* joinSecret)
{
    auto secret = FString(joinSecret);
    UE_LOG(Discord, Log, TEXT("Discord join %s"), *secret);
    if (self) {
        self->OnJoin.Broadcast(secret);
    }
}

static void SpectateGameHandler(const char* spectateSecret)
{
    auto secret = FString(spectateSecret);
    UE_LOG(Discord, Log, TEXT("Discord spectate %s"), *secret);
    if (self) {
        self->OnSpectate.Broadcast(secret);
    }
}

void UDiscordRpc::Initialize(const FString& applicationId, bool autoRegister)
{
    self = this;
    IsConnected = false;
    DiscordEventHandlers handlers{};
    handlers.ready = ReadyHandler;
    handlers.disconnected = DisconnectHandler;
    handlers.errored = ErroredHandler;
    if (OnJoin.IsBound()) {
        handlers.joinGame = JoinGameHandler;
    }
    if (OnSpectate.IsBound()) {
        handlers.spectateGame = SpectateGameHandler;
    }
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
