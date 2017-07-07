#include "discord-rpc.h"

#include "connection.h"
#include "yolojson.h"

static RpcConnection* MyConnection = nullptr;
static char ApplicationId[64]{};
static DiscordEventHandlers Handlers{};
static bool wasJustConnected = false;
static bool wasJustDisconnected = false;

extern "C" void Discord_Initialize(const char* applicationId, DiscordEventHandlers* handlers)
{
    StringCopy(ApplicationId, applicationId, sizeof(ApplicationId));
    if (handlers) {
        Handlers = *handlers;
    }
    else {
        Handlers = {};
    }

    MyConnection = RpcConnection::Create();
    MyConnection->onConnect = []() { wasJustConnected = true; };
    MyConnection->onDisconnect = []() { wasJustDisconnected = true; };
    MyConnection->Open();
}

extern "C" void Discord_Shutdown()
{
    Handlers = {};
    MyConnection->Close();
    RpcConnection::Destroy(MyConnection);
}

extern "C" void Discord_UpdatePresence(const DiscordRichPresence* presence)
{
    auto frame = MyConnection->GetNextFrame();
    char* jsonWrite = frame->message;
    JsonWriteRichPresenceObj(jsonWrite, presence);
    frame->length = sizeof(uint32_t) + (jsonWrite - frame->message);
    MyConnection->WriteFrame(frame);
}

extern "C" void Discord_Update()
{
    // check for messages
    // todo

    // fire callbacks
    if (wasJustDisconnected && Handlers.disconnected) {
        wasJustDisconnected = false;
        Handlers.disconnected();
    }

    if (wasJustConnected && Handlers.ready) {
        wasJustConnected = false;
        Handlers.ready();
    }
}
