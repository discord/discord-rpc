#include "discord-rpc.h"

#include "connection.h"
#include "yolojson.h"

#include <stdio.h>

static RpcConnection* MyConnection = nullptr;
static char ApplicationId[64]{};
static DiscordEventHandlers Handlers{};
static bool WasJustConnected = false;
static bool WasJustDisconnected = false;
static int LastErrorCode = 0;
static const char* LastErrorMessage = "";

extern "C" void Discord_Initialize(const char* applicationId, DiscordEventHandlers* handlers)
{
    if (handlers) {
        Handlers = *handlers;
    }
    else {
        Handlers = {};
    }

    MyConnection = RpcConnection::Create(applicationId);
    MyConnection->onConnect = []() { WasJustConnected = true; };
    MyConnection->onDisconnect = [](int errorCode, const char* message) {
        LastErrorCode = errorCode;
        LastErrorMessage = message;
        WasJustDisconnected = true;
    };
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
    frame->opcode = OPCODE::FRAME;
    char* jsonWrite = frame->message;
    JsonWriteRichPresenceObj(jsonWrite, presence);
    frame->length = jsonWrite - frame->message;
    MyConnection->WriteFrame(frame);
}

extern "C" void Discord_Update()
{
    while (auto frame = MyConnection->Read()) {
        printf("got a message %d, %d, %s\n", frame->opcode, frame->length, frame->message);
    }

    // fire callbacks
    if (WasJustDisconnected && Handlers.disconnected) {
        WasJustDisconnected = false;
        Handlers.disconnected(LastErrorCode, LastErrorMessage);
    }

    if (WasJustConnected && Handlers.ready) {
        WasJustConnected = false;
        Handlers.ready();
    }
}
