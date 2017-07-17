#pragma once

#include "connection.h"

struct RpcConnection {
    enum class Opcode : uint32_t {
        Handshake = 0,
        Frame = 1,
        Close = 2,
    };

    struct MessageFrame {
        Opcode opcode;
        uint32_t length;
        char message[64 * 1024 - 8];
    };

    BaseConnection* connection{nullptr};
    void (*onConnect)(){nullptr};
    void (*onDisconnect)(int errorCode, const char* message){nullptr};
    char appId[64]{};

    static RpcConnection* Create(const char* applicationId);
    static void Destroy(RpcConnection*&);

    void Open();
    void Close();
    void Write(const void* data, size_t length);
    bool Read(void* data, size_t& length);
};