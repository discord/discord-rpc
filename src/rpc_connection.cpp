#include "rpc_connection.h"
#include "yolojson.h"

#include <atomic>

static const int RpcVersion = 1;
static RpcConnection Instance;

/*static*/ RpcConnection* RpcConnection::Create(const char* applicationId)
{
    Instance.connection = BaseConnection::Create();
    StringCopy(Instance.appId, applicationId, sizeof(Instance.appId));
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
            state = State::Connecting;
        }
        else {
            return;
        }
    }

    sendFrame.opcode = Opcode::Handshake;
    char* json = sendFrame.message;
    JsonWriteHandshakeObj(json, RpcVersion, appId);
    sendFrame.length = json - sendFrame.message;
        
    if (connection->Write(&sendFrame, sizeof(MessageFrameHeader) + sendFrame.length)) {
        state = State::Connected;
        if (onConnect) {
            onConnect();
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

bool RpcConnection::Read(rapidjson::Document& message)
{
    if (state != State::Connected) {
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
            StringCopy(lastErrorMessage, m.GetString(), sizeof(lastErrorMessage));
            Close();
            return false;
        }
        case Opcode::Frame:
            message.ParseInsitu(readFrame.message);
            return true;
        case Opcode::Ping:
        {
            readFrame.opcode = Opcode::Pong;
            if (!connection->Write(&readFrame, sizeof(MessageFrameHeader) + readFrame.length)) {
                Close();
            }
            break;
        }
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
