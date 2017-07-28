#include "discord-rpc.h"

#include "backoff.h"
#include "discord-register.h"
#include "rpc_connection.h"
#include "serialization.h"

#include <atomic>
#include <chrono>
#include <mutex>

#ifndef DISCORD_DISABLE_IO_THREAD
#include <condition_variable>
#include <thread>
#endif

constexpr size_t MaxMessageSize{16 * 1024};
constexpr size_t MessageQueueSize{8};

struct QueuedMessage {
    size_t length;
    char buffer[MaxMessageSize];

    void Copy(const QueuedMessage& other)
    {
        length = other.length;
        if (length) {
            memcpy(buffer, other.buffer, length);
        }
    }
};

static RpcConnection* Connection{nullptr};
static DiscordEventHandlers Handlers{};
static std::atomic_bool WasJustConnected{false};
static std::atomic_bool WasJustDisconnected{false};
static std::atomic_bool GotErrorMessage{false};
static std::atomic_bool WasJoinGame{false};
static std::atomic_bool WasSpectateGame{false};
static char JoinGameSecret[256];
static char SpectateGameSecret[256];
static int LastErrorCode{0};
static char LastErrorMessage[256];
static int LastDisconnectErrorCode{0};
static char LastDisconnectErrorMessage[256];
static std::mutex PresenceMutex;
static QueuedMessage QueuedPresence{};
static QueuedMessage SendQueue[MessageQueueSize]{};
static std::atomic_uint SendQueueNextAdd{0};
static std::atomic_uint SendQueueNextSend{0};
static std::atomic_uint SendQueuePendingSends{0};
// We want to auto connect, and retry on failure, but not as fast as possible. This does expoential
// backoff from 0.5 seconds to 1 minute
static Backoff ReconnectTimeMs(500, 60 * 1000);
static auto NextConnect{std::chrono::system_clock::now()};
static int Pid{0};
static int Nonce{1};

#ifndef DISCORD_DISABLE_IO_THREAD
static std::atomic_bool KeepRunning{true};
static std::mutex WaitForIOMutex;
static std::condition_variable WaitForIOActivity;
static std::thread IoThread;
#endif // DISCORD_DISABLE_IO_THREAD

static void UpdateReconnectTime()
{
    NextConnect = std::chrono::system_clock::now() +
      std::chrono::duration<int64_t, std::milli>{ReconnectTimeMs.nextDelay()};
}

static QueuedMessage* SendQueueGetNextAddMessage()
{
    // if we are not connected, let's not batch up stale messages for later
    if (!Connection || !Connection->IsOpen()) {
        return nullptr;
    }

    // if we are falling behind, bail
    if (SendQueuePendingSends.load() >= MessageQueueSize) {
        return nullptr;
    }
    auto index = (SendQueueNextAdd++) % MessageQueueSize;
    return &SendQueue[index];
}
static QueuedMessage* SendQueueGetNextSendMessage()
{
    auto index = (SendQueueNextSend++) % MessageQueueSize;
    return &SendQueue[index];
}
static void SendQueueCommitMessage()
{
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

        for (;;) {
            JsonDocument message;

            if (!Connection->Read(message)) {
                break;
            }

            const char* evtName = GetStrMember(&message, "evt");
            const char* nonce = GetStrMember(&message, "nonce");

            if (nonce) {
                // in responses only -- should use to match up response when needed.

                if (evtName && strcmp(evtName, "ERROR") == 0) {
                    auto data = GetObjMember(&message, "data");
                    LastErrorCode = GetIntMember(data, "code");
                    StringCopy(LastErrorMessage, GetStrMember(data, "message", ""));
                    GotErrorMessage.store(true);
                }
            }
            else {
                // should have evt == name of event, optional data
                if (evtName == nullptr) {
                    continue;
                }

                if (strcmp(evtName, "GAME_JOIN") == 0) {
                    auto data = GetObjMember(&message, "data");
                    auto secret = GetStrMember(data, "secret");
                    if (secret) {
                        StringCopy(JoinGameSecret, secret);
                        WasJoinGame.store(true);
                    }
                }
                else if (strcmp(evtName, "GAME_SPECTATE") == 0) {
                    auto data = GetObjMember(&message, "data");
                    auto secret = GetStrMember(data, "secret");
                    if (secret) {
                        StringCopy(SpectateGameSecret, secret);
                        WasSpectateGame.store(true);
                    }
                }
            }
        }

        // writes
        if (QueuedPresence.length) {
            QueuedMessage local;
            PresenceMutex.lock();
            local.Copy(QueuedPresence);
            QueuedPresence.length = 0;
            PresenceMutex.unlock();
            if (!Connection->Write(local.buffer, local.length)) {
                // if we fail to send, requeue
                PresenceMutex.lock();
                QueuedPresence.Copy(local);
                PresenceMutex.unlock();
            }
        }

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

bool RegisterForEvent(const char* evtName)
{
    auto qmessage = SendQueueGetNextAddMessage();
    if (qmessage) {
        qmessage->length =
          JsonWriteSubscribeCommand(qmessage->buffer, sizeof(qmessage->buffer), Nonce++, evtName);
        SendQueueCommitMessage();
        SignalIOActivity();
        return true;
    }
    return false;
}

extern "C" void Discord_Initialize(const char* applicationId,
                                   DiscordEventHandlers* handlers,
                                   int autoRegister)
{
    if (autoRegister) {
        Discord_Register(applicationId);
    }

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

        if (Handlers.joinGame) {
            RegisterForEvent("GAME_JOIN");
        }

        if (Handlers.spectateGame) {
            RegisterForEvent("GAME_SPECTATE");
        }
    };
    Connection->onDisconnect = [](int err, const char* message) {
        LastDisconnectErrorCode = err;
        StringCopy(LastDisconnectErrorMessage, message);
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
    PresenceMutex.lock();
    QueuedPresence.length = JsonWriteRichPresenceObj(
      QueuedPresence.buffer, sizeof(QueuedPresence.buffer), Nonce++, Pid, presence);
    PresenceMutex.unlock();
    SignalIOActivity();
}

extern "C" void Discord_RunCallbacks()
{
    if (GotErrorMessage.exchange(false) && Handlers.errored) {
        Handlers.errored(LastErrorCode, LastErrorMessage);
    }

    if (WasJustDisconnected.exchange(false) && Handlers.disconnected) {
        Handlers.disconnected(LastDisconnectErrorCode, LastDisconnectErrorMessage);
    }

    if (WasJustConnected.exchange(false) && Handlers.ready) {
        Handlers.ready();
    }

    if (WasJoinGame.exchange(false) && Handlers.joinGame) {
        Handlers.joinGame(JoinGameSecret);
    }

    if (WasSpectateGame.exchange(false) && Handlers.spectateGame) {
        Handlers.spectateGame(SpectateGameSecret);
    }
}
