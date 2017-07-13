#include "discord-rpc.h"

#include "connection.h"
#include "yolojson.h"
#include "rapidjson/document.h"

#include <stdio.h>

static RpcConnection* MyConnection = nullptr;
static char ApplicationId[64]{};
static DiscordEventHandlers Handlers{};
static bool WasJustConnected = false;
static bool WasJustDisconnected = false;
static int LastErrorCode = 0;
static char LastErrorMessage[256];

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
    MyConnection->onDisconnect = []() { WasJustDisconnected = true; };
    MyConnection->Open();
}

extern "C" void Discord_Shutdown()
{
    Handlers = {};
    MyConnection->onConnect = nullptr;
    MyConnection->onDisconnect = nullptr;
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
        rapidjson::Document d;
        if (frame->length > 0) {
            d.ParseInsitu(frame->message);
        }

        switch (frame->opcode) {
        case OPCODE::HANDSHAKE:
            // does this happen?
            break;
        case OPCODE::CLOSE:
            LastErrorCode = d["code"].GetInt();
            StringCopy(LastErrorMessage, d["code"].GetString(), sizeof(LastErrorMessage));
            MyConnection->Close();
            break;
        case OPCODE::FRAME:
            // todo
            break;
        }
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
