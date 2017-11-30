

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

static void JoinRequestHandler(const DiscordJoinRequest* request)
{
    FDiscordJoinRequestData jr;
    jr.userId = ANSI_TO_TCHAR(request->userId);
    jr.username = ANSI_TO_TCHAR(request->username);
    jr.discriminator = ANSI_TO_TCHAR(request->discriminator);
    jr.avatar = ANSI_TO_TCHAR(request->avatar);
    UE_LOG(Discord, Log, TEXT("Discord join request from %s#%s"), *jr.username, *jr.discriminator);
    if (self) {
        self->OnJoinRequest.Broadcast(jr);
    }
}

void UDiscordRpc::Initialize(const FString& applicationId,
                             bool autoRegister,
                             const FString& optionalSteamId)
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
    if (OnJoinRequest.IsBound()) {
        handlers.joinRequest = JoinRequestHandler;
    }
    auto appId = StringCast<ANSICHAR>(*applicationId);
    auto steamId = StringCast<ANSICHAR>(*optionalSteamId);
    Discord_Initialize(
      (const char*)appId.Get(), &handlers, autoRegister, (const char*)steamId.Get());
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

void UDiscordRpc::UpdatePresence()
{
    DiscordRichPresence rp{};

    auto state = StringCast<ANSICHAR>(*RichPresence.state);
    rp.state = state.Get();

    auto details = StringCast<ANSICHAR>(*RichPresence.details);
    rp.details = details.Get();

    auto largeImageKey = StringCast<ANSICHAR>(*RichPresence.largeImageKey);
    rp.largeImageKey = largeImageKey.Get();

    auto largeImageText = StringCast<ANSICHAR>(*RichPresence.largeImageText);
    rp.largeImageText = largeImageText.Get();

    auto smallImageKey = StringCast<ANSICHAR>(*RichPresence.smallImageKey);
    rp.smallImageKey = smallImageKey.Get();

    auto smallImageText = StringCast<ANSICHAR>(*RichPresence.smallImageText);
    rp.smallImageText = smallImageText.Get();

    auto partyId = StringCast<ANSICHAR>(*RichPresence.partyId);
    rp.partyId = partyId.Get();

    auto matchSecret = StringCast<ANSICHAR>(*RichPresence.matchSecret);
    rp.matchSecret = matchSecret.Get();

    auto joinSecret = StringCast<ANSICHAR>(*RichPresence.joinSecret);
    rp.joinSecret = joinSecret.Get();

    auto spectateSecret = StringCast<ANSICHAR>(*RichPresence.spectateSecret);
    rp.spectateSecret = spectateSecret.Get();

    rp.startTimestamp = RichPresence.startTimestamp;
    rp.endTimestamp = RichPresence.endTimestamp;
    rp.partySize = RichPresence.partySize;
    rp.partyMax = RichPresence.partyMax;
    rp.instance = RichPresence.instance;

    Discord_UpdatePresence(&rp);
}
