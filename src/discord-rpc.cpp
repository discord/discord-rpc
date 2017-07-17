#include "discord-rpc.h"

#include "rpc_connection.h"
#include "yolojson.h"
#include "rapidjson/document.h"

#include <stdio.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <thread>

static RpcConnection* Connection{nullptr};
static char ApplicationId[64]{};
static DiscordEventHandlers Handlers{};
static std::atomic_bool WasJustConnected{false};
static std::atomic_bool WasJustDisconnected{false};
static int LastErrorCode = 0;
static char LastErrorMessage[256];
static std::atomic_bool KeepRunning{true};
static std::mutex WaitForIOMutex;
static std::condition_variable WaitForIOActivity;
static std::thread IoThread;

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
    }
}

void DiscordRpcIo()
{
    printf("Discord io thread start\n");
    const std::chrono::duration<int64_t, std::milli> maxWait{500LL};
    
    while (KeepRunning.load()) {
        Discord_UpdateConnection();

        std::unique_lock<std::mutex> lock(WaitForIOMutex);
        WaitForIOActivity.wait_for(lock, maxWait);
    }
    Connection->Close();
    printf("Discord io thread stop\n");
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
    char jsonBuffer[16 * 1024];
    char* jsonWrite = jsonBuffer;
    JsonWriteRichPresenceObj(jsonWrite, presence);
    size_t length = jsonWrite - jsonBuffer;
    Connection->Write(jsonBuffer, length);
    SignalIOActivity();
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
