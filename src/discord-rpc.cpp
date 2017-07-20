#include "discord-rpc.h"

#include "backoff.h"
#include "rpc_connection.h"
#include "serialization.h"

#include <atomic>
#include <chrono>

#ifndef DISCORD_DISABLE_IO_THREAD
#include <condition_variable>
#include <thread>
#endif

#include "rapidjson/internal/itoa.h"

constexpr size_t MaxMessageSize = 16 * 1024;
constexpr size_t MessageQueueSize = 8;

struct QueuedMessage {
    size_t length;
    char buffer[MaxMessageSize];
};

static RpcConnection* Connection{nullptr};
static char ApplicationId[64]{};
static DiscordEventHandlers Handlers{};
static std::atomic_bool WasJustConnected{false};
static std::atomic_bool WasJustDisconnected{false};
static int LastErrorCode{0};
static char LastErrorMessage[256];
static QueuedMessage SendQueue[MessageQueueSize]{};
static std::atomic_uint SendQueueNextAdd{0};
static std::atomic_uint SendQueueNextSend{0};
static std::atomic_uint SendQueuePendingSends{0};
static Backoff ReconnectTimeMs(500, 60 * 1000);
static auto NextConnect{std::chrono::system_clock::now()};
static int Pid = 0;
static int Nonce = 1;

#ifndef DISCORD_DISABLE_IO_THREAD
static std::atomic_bool KeepRunning{ true };
static std::mutex WaitForIOMutex;
static std::condition_variable WaitForIOActivity;
static std::thread IoThread;
#endif // DISCORD_DISABLE_IO_THREAD

static void UpdateReconnectTime()
{
    NextConnect = std::chrono::system_clock::now() + std::chrono::duration<int64_t, std::milli>{ReconnectTimeMs.nextDelay()};
}

static QueuedMessage* SendQueueGetNextAddMessage() {
    // if we are falling behind, bail
    if (SendQueuePendingSends.load() >= MessageQueueSize) {
        return nullptr;
    }
    auto index = (SendQueueNextAdd++) % MessageQueueSize;
    return &SendQueue[index];
}
static QueuedMessage* SendQueueGetNextSendMessage() {
    auto index = (SendQueueNextSend++) % MessageQueueSize;
    return &SendQueue[index];
}
static void SendQueueCommitMessage() {
    SendQueuePendingSends++;
}

extern "C" void Discord_UpdateConnection()
{
    if (!Connection->IsOpen()) {
        if (std::chrono::system_clock::now() >= NextConnect) {
            UpdateReconnectTime();
            Connection->Open();
        }
    }
    else {
        // reads
        rapidjson::Document message;
        while (Connection->Read(message)) {
            // todo: do something...
            printf("Hey, I got a message\n");
        }

        // writes
        while (SendQueuePendingSends.load()) {
            auto qmessage = SendQueueGetNextSendMessage();
            Connection->Write(qmessage->buffer, qmessage->length);
            --SendQueuePendingSends;
        }
    }
}

#ifndef DISCORD_DISABLE_IO_THREAD
void DiscordRpcIo()
{
    const std::chrono::duration<int64_t, std::milli> maxWait{500LL};
    
    while (KeepRunning.load()) {
        Discord_UpdateConnection();

        std::unique_lock<std::mutex> lock(WaitForIOMutex);
        WaitForIOActivity.wait_for(lock, maxWait);
    }
}
#endif

void SignalIOActivity()
{
#ifndef DISCORD_DISABLE_IO_THREAD
    WaitForIOActivity.notify_all();
#endif
}

extern "C" void Discord_Initialize(const char* applicationId, DiscordEventHandlers* handlers)
{
    Pid = GetProcessId();

    if (handlers) {
        Handlers = *handlers;
    }
    else {
        Handlers = {};
    }

    Connection = RpcConnection::Create(applicationId);
    Connection->onConnect = []() {
        WasJustConnected.exchange(true);
        ReconnectTimeMs.reset();
    };
    Connection->onDisconnect = [](int err, const char* message) {
        LastErrorCode = err;
        StringCopy(LastErrorMessage, message);
        WasJustDisconnected.exchange(true);
        UpdateReconnectTime();
    };

#ifndef DISCORD_DISABLE_IO_THREAD
    IoThread = std::thread(DiscordRpcIo);
#endif
}

extern "C" void Discord_Shutdown()
{
    Connection->onConnect = nullptr;
    Connection->onDisconnect = nullptr;
    Handlers = {};
#ifndef DISCORD_DISABLE_IO_THREAD
    KeepRunning.exchange(false);
    SignalIOActivity();
    if (IoThread.joinable()) {
        IoThread.join();
    }
#endif
    RpcConnection::Destroy(Connection);
}

extern "C" void Discord_UpdatePresence(const DiscordRichPresence* presence)
{
    auto qmessage = SendQueueGetNextAddMessage();
    if (qmessage) {
        char nonce[32]{};
        rapidjson::internal::i32toa(Nonce++, nonce);
        qmessage->length = JsonWriteRichPresenceObj(qmessage->buffer, sizeof(qmessage->buffer), nonce, Pid, presence);
        SendQueueCommitMessage();
        SignalIOActivity();
    }
}

extern "C" void Discord_RunCallbacks()
{
    if (WasJustDisconnected.exchange(false) && Handlers.disconnected) {
        Handlers.disconnected(LastErrorCode, LastErrorMessage);
    }

    if (WasJustConnected.exchange(false) && Handlers.ready) {
        Handlers.ready();
    }
}
