#include "discord-rpc.h"

#include "connection.h"
#include "yolojson.h"

static RpcConnection* MyConnection = nullptr;
static char ApplicationId[64]{};
static DiscordEventHandlers Handlers{};

void Discord_Initialize(const char* applicationId, DiscordEventHandlers* handlers)
{
    StringCopy(ApplicationId, applicationId, sizeof(ApplicationId));
    if (handlers) {
        Handlers = *handlers;
    }
    else {
        Handlers = {};
    }

    MyConnection = RpcConnection::Create();
    MyConnection->onConnect = Handlers.ready;
    MyConnection->onDisconnect = Handlers.disconnected;
    MyConnection->Open();
}

void Discord_Shutdown()
{
    Handlers = {};
    MyConnection->Close();
    RpcConnection::Destroy(MyConnection);
}

void Discord_UpdatePresence(const DiscordRichPresence* presence)
{
    auto frame = MyConnection->GetNextFrame();
    char* jsonWrite = frame->message;
    JsonWriteRichPresenceObj(jsonWrite, presence);
    frame->length = sizeof(uint32_t) + (jsonWrite - frame->message);
    MyConnection->WriteFrame(frame);
}
