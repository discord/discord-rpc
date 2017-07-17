#include "rpc_connection.h"
#include "yolojson.h"

#include <atomic>

static const int RpcVersion = 1;
static RpcConnection Instance;
static const size_t SendQueueSize = 4;
static RpcConnection::MessageFrame SendQueue[SendQueueSize];
static std::atomic_uint SendQueueNext = 0;

static RpcConnection::MessageFrame* NextSendFrame() {
    auto index = (SendQueueNext++) % SendQueueSize;
    return &SendQueue[index];
}

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

    auto handshakeFrame = NextSendFrame();
    handshakeFrame->opcode = Opcode::Handshake;
    char* json = handshakeFrame->message;
    JsonWriteHandshakeObj(json, RpcVersion, appId);
    handshakeFrame->length = json - handshakeFrame->message;
        
    if (connection->Write(handshakeFrame, sizeof(MessageFrameHeader) + handshakeFrame->length)) {
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

void RpcConnection::Write(const void* data, size_t length)
{
    auto frame = NextSendFrame();
    frame->opcode = Opcode::Frame;
    memcpy(frame->message, data, length);
    frame->length = length;
    if (!connection->Write(frame, sizeof(MessageFrameHeader) + length)) {
        Close();
    }
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
            message.ParseInsitu(readFrame.message);
        }

        switch (readFrame.opcode) {
        case Opcode::Close:
        {
            lastErrorCode = message["code"].GetInt();
            const auto& m = message["message"];
            StringCopy(lastErrorMessage, m.GetString(), sizeof(lastErrorMessage));
            Close();
            return false;
        }
        case Opcode::Frame:
            return true;
        case Opcode::Ping:
        {
            MessageFrameHeader frame{ Opcode::Pong, 0 };
            if (!connection->Write(&frame, sizeof(MessageFrameHeader))) {
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
