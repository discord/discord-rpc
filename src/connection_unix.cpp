#include "connection.h"

/*static*/ RpcConnection* RpcConnection::Create(const char* applicationId)
{
    return nullptr;
}

/*static*/ void RpcConnection::Destroy(RpcConnection*&)
{
    
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
    return nullptr;
}

void RpcConnection::WriteFrame(RpcMessageFrame* frame)
{
    
}
