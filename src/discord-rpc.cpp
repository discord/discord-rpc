#include "discord-rpc.h"

#include "rpc_connection.h"
#include "yolojson.h"
#include "rapidjson/document.h"

#include <stdio.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <thread>

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
static std::atomic_bool KeepRunning{true};
static std::mutex WaitForIOMutex;
static std::condition_variable WaitForIOActivity;
static std::thread IoThread;
static QueuedMessage SendQueue[MessageQueueSize]{};
static std::atomic_uint SendQueueNextAdd{0};
static std::atomic_uint SendQueueNextSend{0};
static std::atomic_uint SendQueuePendingSends{0};

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

void Discord_UpdateConnection()
{
    if (!Connection->IsOpen()) {
        Connection->Open();
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

void DiscordRpcIo()
{
    const std::chrono::duration<int64_t, std::milli> maxWait{500LL};
    
    while (KeepRunning.load()) {
        Discord_UpdateConnection();

        std::unique_lock<std::mutex> lock(WaitForIOMutex);
        WaitForIOActivity.wait_for(lock, maxWait);
    }
}

void SignalIOActivity()
{
    WaitForIOActivity.notify_all();
}

extern "C" void Discord_Initialize(const char* applicationId, DiscordEventHandlers* handlers)
{
    if (handlers) {
        Handlers = *handlers;
    }
    else {
        Handlers = {};
    }

    Connection = RpcConnection::Create(applicationId);
    Connection->onConnect = []() {
        WasJustConnected.exchange(true);
    };
    Connection->onDisconnect = [](int err, const char* message) {
        LastErrorCode = err;
        StringCopy(LastErrorMessage, message, sizeof(LastErrorMessage));
        WasJustDisconnected.exchange(true);
    };

    IoThread = std::thread(DiscordRpcIo);
}

extern "C" void Discord_Shutdown()
{
    Connection->onConnect = nullptr;
    Connection->onDisconnect = nullptr;
    Handlers = {};
    KeepRunning.exchange(false);
    SignalIOActivity();
    if (IoThread.joinable()) {
        IoThread.join();
    }
    RpcConnection::Destroy(Connection);
}

extern "C" void Discord_UpdatePresence(const DiscordRichPresence* presence)
{
    auto qmessage = SendQueueGetNextAddMessage();
    if (qmessage) {
        char* jsonWrite = qmessage->buffer;
        JsonWriteRichPresenceObj(jsonWrite, presence);
        qmessage->length = jsonWrite - qmessage->buffer;
        SendQueueCommitMessage();
        SignalIOActivity();
    }
}

extern "C" void Discord_Update()
{
    if (WasJustDisconnected.exchange(false) && Handlers.disconnected) {
        Handlers.disconnected(LastErrorCode, LastErrorMessage);
    }

    if (WasJustConnected.exchange(false) && Handlers.ready) {
        Handlers.ready();
    }
}
