#include "rpc_connection.h"

RpcConnection Instance;

/*static*/ RpcConnection* RpcConnection::Create(const char* applicationId)
{
    Instance.connection = BaseConnection::Create();
    StringCopy(Instance.appId, applicationId, sizeof(Instance.appId));
    return &Instance;
}

/*static*/ void RpcConnection::Destroy(RpcConnection*& c)
{
    BaseConnection::Destroy(c->connection);
}

