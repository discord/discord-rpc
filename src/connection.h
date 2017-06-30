#pragma once

// This is to wrap the platform specific kinds of connect/read/write.

#include <stdint.h>

struct RpcMessageFrame {
    uint32_t length;
    char message[64 * 1024 - sizeof(uint32_t)];
};

struct RpcConnection {
    void (*onConnect)() = nullptr;
    void (*onDisconnect)() = nullptr;

    static RpcConnection* Create();
    static void Destroy(RpcConnection*&);
    void Open();
    void Close();
    void Write(const void* data, size_t length);
    
    RpcMessageFrame* GetNextFrame();
    void WriteFrame(RpcMessageFrame* frame);
};
