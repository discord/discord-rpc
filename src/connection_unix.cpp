#include "connection.h"

const int RpcVersion = 1;
const int NumFrames = 4;

struct RpcConnectionUnix : public RpcConnection {
    int pipe{-1};
    RpcMessageFrame frames[NumFrames];
    int nextFrame{0};
};

/*static*/ RpcConnection* RpcConnection::Create(const char* applicationId)
{
    return new RpcConnectionUnix;
}

/*static*/ void RpcConnection::Destroy(RpcConnection*& c)
{
    auto self = reinterpret_cast<RpcConnectionUnix*&>(c);
    delete self;
    c = nullptr;
}

void RpcConnection::Open()
{
    
}

void RpcConnection::Close()
{
    
}

void RpcConnection::Write(const void* data, size_t length)
{
    
}

RpcMessageFrame* RpcConnection::Read()
{
    return nullptr;
}

RpcMessageFrame* RpcConnection::GetNextFrame()
{
    auto self = reinterpret_cast<RpcConnectionUnix*>(this);
    auto result = &(self->frames[self->nextFrame]);
    self->nextFrame = (self->nextFrame + 1) % NumFrames;
    return result;
}

void RpcConnection::WriteFrame(RpcMessageFrame* frame)
{
    
}
