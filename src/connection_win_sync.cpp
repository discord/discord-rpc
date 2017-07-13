#include "connection.h"

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#define NOMCX
#define NOSERVICE
#define NOIME
#include <windows.h>

#include "yolojson.h"

const int RpcVersion = 1;
const int NumFrames = 4;

struct WinRpcConnection : public RpcConnection {
    HANDLE pipe{INVALID_HANDLE_VALUE};
    RpcMessageFrame readFrame;
    RpcMessageFrame frames[NumFrames];
    int nextFrame{0};
};

static const wchar_t* PipeName = L"\\\\?\\pipe\\discord-ipc";

/*static*/ RpcConnection* RpcConnection::Create(const char* applicationId)
{
    auto connection = new WinRpcConnection;
    StringCopy(connection->appId, applicationId, sizeof(connection->appId));
    return connection;
}

/*static*/ void RpcConnection::Destroy(RpcConnection*& c)
{
    auto self = reinterpret_cast<WinRpcConnection*>(c);
    delete self;
    c = nullptr;
}

void RpcConnection::Open()
{
    auto self = reinterpret_cast<WinRpcConnection*>(this);
    for (;;) {
        self->pipe = ::CreateFileW(PipeName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (self->pipe != INVALID_HANDLE_VALUE) {
            break;
        }

        if (GetLastError() != ERROR_PIPE_BUSY) {
            printf("Could not open pipe. Error: %d\n", GetLastError());
            return;
        }

        if (!WaitNamedPipeW(PipeName, 10000)) {
            printf("Could not open pipe: 10 second wait timed out.\n");
            return;
        }
    }

    RpcMessageFrame* frame = GetNextFrame();
    frame->opcode = OPCODE::HANDSHAKE;
    char* msg = frame->message;
    JsonWriteHandshakeObj(msg, RpcVersion, appId);
    frame->length = msg - frame->message;
    WriteFrame(frame);

    if (self->onConnect) {
        self->onConnect();
    }
}

void RpcConnection::Close()
{
    auto self = reinterpret_cast<WinRpcConnection*>(this);
    ::CloseHandle(self->pipe);
    self->pipe = INVALID_HANDLE_VALUE;
    if (self->onDisconnect) {
        self->onDisconnect();
    }
}

void RpcConnection::Write(const void* data, size_t length)
{
    auto self = reinterpret_cast<WinRpcConnection*>(this);
    const int retries = 3;
    for (int i = 0; i < retries; ++i) {
        if (self->pipe == INVALID_HANDLE_VALUE) {
            self->Open();
            if (self->pipe == INVALID_HANDLE_VALUE) {
                break;
            }
        }
        BOOL success = ::WriteFile(self->pipe, data, length, nullptr, nullptr);
        if (success) {
            break;
        }

        /* hmm
        RpcMessageFrame* frame = self->Read();
        if (frame) {
            self->HandleError(frame);
        }
        */

        self->Close();
    }
}

RpcMessageFrame* RpcConnection::Read()
{
    auto self = reinterpret_cast<WinRpcConnection*>(this);
    DWORD bytesAvailable = 0;
    if (::PeekNamedPipe(self->pipe, nullptr, 0, nullptr, &bytesAvailable, nullptr) && bytesAvailable > 8) {
        if (::ReadFile(self->pipe, &self->readFrame, 8, nullptr, nullptr) != TRUE) {
            return nullptr;
        }

        if (self->readFrame.length > 0) {
            if (::ReadFile(self->pipe, &self->readFrame.message, self->readFrame.length, nullptr, nullptr) != TRUE) {
                return nullptr;
            }
            self->readFrame.message[self->readFrame.length] = 0;
        }

        return &self->readFrame;
    }
    return nullptr;
}

RpcMessageFrame* RpcConnection::GetNextFrame()
{
    auto self = reinterpret_cast<WinRpcConnection*>(this);
    auto result = &(self->frames[self->nextFrame]);
    self->nextFrame = (self->nextFrame + 1) % NumFrames;
    return result;
}

void RpcConnection::WriteFrame(RpcMessageFrame* frame)
{
    auto self = reinterpret_cast<WinRpcConnection*>(this);
    self->Write(frame, 8 + frame->length);
}
