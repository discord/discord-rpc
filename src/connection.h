#pragma once

// This is to wrap the platform specific kinds of connect/read/write.

#include <stdint.h>

enum class OPCODE : uint32_t {
    HANDSHAKE = 0,
    FRAME = 1,
    CLOSE = 2,
};

struct RpcMessageFrame {
    OPCODE opcode;
    uint32_t length;
    char message[64 * 1024 - 8];
};

struct RpcConnection {
    void (*onConnect)() = nullptr;
    void (*onDisconnect)() = nullptr;
    char appId[64];

    static RpcConnection* Create(const char* applicationId);
    static void Destroy(RpcConnection*&);
    void Open();
    void Close();
    void Write(const void* data, size_t length);
    RpcMessageFrame* Read();
    
    RpcMessageFrame* GetNextFrame();
    void WriteFrame(RpcMessageFrame* frame);
};
