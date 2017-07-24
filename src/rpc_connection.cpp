#include "rpc_connection.h"
#include "serialization.h"

#include <atomic>

static const int RpcVersion = 1;
static RpcConnection Instance;

/*static*/ RpcConnection* RpcConnection::Create(const char* applicationId)
{
    Instance.connection = BaseConnection::Create();
    StringCopy(Instance.appId, applicationId);
    return &Instance;
}

/*static*/ void RpcConnection::Destroy(RpcConnection*& c)
{
    c->Close();
    BaseConnection::Destroy(c->connection);
}

void RpcConnection::Open()
{
    if (state == State::Connected) {
        return;
    }

    if (state == State::Disconnected) {
        if (connection->Open()) {
        }
        else {
            return;
        }
    }

    if (state == State::SentHandshake) {
        char parseBuffer[32 * 1024];
        PoolAllocator pa(parseBuffer, sizeof(parseBuffer));
        StackAllocator sa;
        JsonDocument message(rapidjson::kObjectType, &pa, sizeof(sa.fixedBuffer_), &sa);
        if (Read(message)) {
            auto cmd = message.FindMember("cmd");
            if (cmd == message.MemberEnd() || !cmd->value.IsString()) {
                return;
            }
            auto evt = message.FindMember("evt");
            if (evt == message.MemberEnd() || !evt->value.IsString()) {
                return;
            }
            if (!strcmp(cmd->value.GetString(), "DISPATCH") && !strcmp(evt->value.GetString(), "READY")) {
                state = State::Connected;
                if (onConnect) {
                    onConnect();
                }
            }
        }
    }
    else {
        sendFrame.opcode = Opcode::Handshake;
        sendFrame.length = JsonWriteHandshakeObj(sendFrame.message, sizeof(sendFrame.message), RpcVersion, appId);

        if (connection->Write(&sendFrame, sizeof(MessageFrameHeader) + sendFrame.length)) {
            state = State::SentHandshake;
        }
        else {
            Close();
        }
    }
}

void RpcConnection::Close()
{
    if (onDisconnect && state == State::Connected) {
        onDisconnect(lastErrorCode, lastErrorMessage);
    }
    connection->Close();
    state = State::Disconnected;
}

bool RpcConnection::Write(const void* data, size_t length)
{
    sendFrame.opcode = Opcode::Frame;
    memcpy(sendFrame.message, data, length);
    sendFrame.length = length;
    if (!connection->Write(&sendFrame, sizeof(MessageFrameHeader) + length)) {
        Close();
        return false;
    }
    return true;
}

bool RpcConnection::Read(JsonDocument& message)
{
    if (state != State::Connected && state != State::SentHandshake) {
        return false;
    }
    MessageFrame readFrame;
    for (;;) {
        bool didRead = connection->Read(&readFrame, sizeof(MessageFrameHeader));
        if (!didRead) {
            return false;
        }

        if (readFrame.length > 0) {
            didRead = connection->Read(readFrame.message, readFrame.length);
            if (!didRead) {
                lastErrorCode = -2;
                StringCopy(lastErrorMessage, "Partial data in frame");
                Close();
                return false;
            }
            readFrame.message[readFrame.length] = 0;
        }

        switch (readFrame.opcode) {
        case Opcode::Close:
        {
            message.ParseInsitu(readFrame.message);
            lastErrorCode = message["code"].GetInt();
            const auto& m = message["message"];
            StringCopy(lastErrorMessage, m.GetString());
            Close();
            return false;
        }
        case Opcode::Frame:
            message.ParseInsitu(readFrame.message);
            return true;
        case Opcode::Ping:
            readFrame.opcode = Opcode::Pong;
            if (!connection->Write(&readFrame, sizeof(MessageFrameHeader) + readFrame.length)) {
                Close();
            }
            break;
        case Opcode::Pong:
            break;
        default:
            // something bad happened
            lastErrorCode = -1;
            StringCopy(lastErrorMessage, "Bad ipc frame");
            Close();
            return false;
        }
    }
}
