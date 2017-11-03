#include "discord-rpc.h"

#include "backoff.h"
#include "discord_register.h"
#include "msg_queue.h"
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
constexpr size_t JoinQueueSize{8};

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

struct JoinRequest {
    // snowflake (64bit int), turned into a ascii decimal string, at most 20 chars +1 null
    // terminator = 21
    char userId[22];
    // 32 unicode glyphs is max name size => 4 bytes per glyph in the worst case, +1 for null
    // terminator = 129
    char username[130];
    // optional 'a_' + md5 hex digest (32 bytes) + null terminator = 35
    char avatar[36];
    // +1 on each because: it's even / I'm paranoid
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
static MsgQueue<QueuedMessage, MessageQueueSize> SendQueue;
static MsgQueue<JoinRequest, JoinQueueSize> JoinAskQueue;

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

#ifdef DISCORD_DISABLE_IO_THREAD
extern "C" DISCORD_EXPORT void Discord_UpdateConnection(void)
#else
static void Discord_UpdateConnection(void)
#endif
{
    if (!Connection) {
        return;
    }

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

                if (strcmp(evtName, "ACTIVITY_JOIN") == 0) {
                    auto data = GetObjMember(&message, "data");
                    auto secret = GetStrMember(data, "secret");
                    if (secret) {
                        StringCopy(JoinGameSecret, secret);
                        WasJoinGame.store(true);
                    }
                }
                else if (strcmp(evtName, "ACTIVITY_SPECTATE") == 0) {
                    auto data = GetObjMember(&message, "data");
                    auto secret = GetStrMember(data, "secret");
                    if (secret) {
                        StringCopy(SpectateGameSecret, secret);
                        WasSpectateGame.store(true);
                    }
                }
                else if (strcmp(evtName, "ACTIVITY_JOIN_REQUEST") == 0) {
                    auto data = GetObjMember(&message, "data");
                    auto user = GetObjMember(data, "user");
                    auto userId = GetStrMember(user, "id");
                    auto username = GetStrMember(user, "username");
                    auto avatar = GetStrMember(user, "avatar");
                    auto joinReq = JoinAskQueue.GetNextAddMessage();
                    if (userId && username && joinReq) {
                        StringCopy(joinReq->userId, userId);
                        StringCopy(joinReq->username, username);
                        if (avatar) {
                            StringCopy(joinReq->avatar, avatar);
                        }
                        else {
                            joinReq->avatar[0] = 0;
                        }
                        JoinAskQueue.CommitAdd();
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

        while (SendQueue.HavePendingSends()) {
            auto qmessage = SendQueue.GetNextSendMessage();
            Connection->Write(qmessage->buffer, qmessage->length);
            SendQueue.CommitSend();
        }
    }
}

#ifndef DISCORD_DISABLE_IO_THREAD
static void DiscordRpcIo(void)
{
    const std::chrono::duration<int64_t, std::milli> maxWait{500LL};

    while (KeepRunning.load()) {
        Discord_UpdateConnection();

        std::unique_lock<std::mutex> lock(WaitForIOMutex);
        WaitForIOActivity.wait_for(lock, maxWait);
    }
}
#endif

static void SignalIOActivity()
{
#ifndef DISCORD_DISABLE_IO_THREAD
    WaitForIOActivity.notify_all();
#endif
}

static bool RegisterForEvent(const char* evtName)
{
    auto qmessage = SendQueue.GetNextAddMessage();
    if (qmessage) {
        qmessage->length =
          JsonWriteSubscribeCommand(qmessage->buffer, sizeof(qmessage->buffer), Nonce++, evtName);
        SendQueue.CommitAdd();
        SignalIOActivity();
        return true;
    }
    return false;
}

extern "C" DISCORD_EXPORT void Discord_Initialize(const char* applicationId,
                                                  DiscordEventHandlers* handlers,
                                                  int autoRegister,
                                                  const char* optionalSteamId)
{
    if (autoRegister) {
        if (optionalSteamId && optionalSteamId[0]) {
            Discord_RegisterSteamGame(applicationId, optionalSteamId);
        }
        else {
            Discord_Register(applicationId, nullptr);
        }
    }

    Pid = GetProcessId();

    if (handlers) {
        Handlers = *handlers;
    }
    else {
        Handlers = {};
    }

    if (Connection) {
        return;
    }

    Connection = RpcConnection::Create(applicationId);
    Connection->onConnect = []() {
        WasJustConnected.exchange(true);
        ReconnectTimeMs.reset();

        if (Handlers.joinGame) {
            RegisterForEvent("ACTIVITY_JOIN");
        }

        if (Handlers.spectateGame) {
            RegisterForEvent("ACTIVITY_SPECTATE");
        }

        if (Handlers.joinRequest) {
            RegisterForEvent("ACTIVITY_JOIN_REQUEST");
        }
    };
    Connection->onDisconnect = [](int err, const char* message) {
        LastDisconnectErrorCode = err;
        StringCopy(LastDisconnectErrorMessage, message);
        WasJustDisconnected.exchange(true);
        UpdateReconnectTime();
    };

#ifndef DISCORD_DISABLE_IO_THREAD
    KeepRunning.store(true);
    IoThread = std::thread(DiscordRpcIo);
#endif
}

extern "C" DISCORD_EXPORT void Discord_Shutdown()
{
    if (!Connection) {
        return;
    }
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

extern "C" DISCORD_EXPORT void Discord_UpdatePresence(const DiscordRichPresence* presence)
{
    PresenceMutex.lock();
    QueuedPresence.length = JsonWriteRichPresenceObj(
      QueuedPresence.buffer, sizeof(QueuedPresence.buffer), Nonce++, Pid, presence);
    PresenceMutex.unlock();
    SignalIOActivity();
}

extern "C" DISCORD_EXPORT void Discord_Respond(const char* userId, /* DISCORD_REPLY_ */ int reply)
{
    // if we are not connected, let's not batch up stale messages for later
    if (!Connection || !Connection->IsOpen()) {
        return;
    }
    auto qmessage = SendQueue.GetNextAddMessage();
    if (qmessage) {
        qmessage->length =
          JsonWriteJoinReply(qmessage->buffer, sizeof(qmessage->buffer), userId, reply, Nonce++);
        SendQueue.CommitAdd();
        SignalIOActivity();
    }
}

extern "C" DISCORD_EXPORT void Discord_RunCallbacks()
{
    // Note on some weirdness: internally we might connect, get other signals, disconnect any number
    // of times inbetween calls here. Externally, we want the sequence to seem sane, so any other
    // signals are book-ended by calls to ready and disconnect.

    if (!Connection) {
        return;
    }

    bool wasDisconnected = WasJustDisconnected.exchange(false);
    bool isConnected = Connection->IsOpen();

    if (isConnected) {
        // if we are connected, disconnect cb first
        if (wasDisconnected && Handlers.disconnected) {
            Handlers.disconnected(LastDisconnectErrorCode, LastDisconnectErrorMessage);
        }
    }

    if (WasJustConnected.exchange(false) && Handlers.ready) {
        Handlers.ready();
    }

    if (GotErrorMessage.exchange(false) && Handlers.errored) {
        Handlers.errored(LastErrorCode, LastErrorMessage);
    }

    if (WasJoinGame.exchange(false) && Handlers.joinGame) {
        Handlers.joinGame(JoinGameSecret);
    }

    if (WasSpectateGame.exchange(false) && Handlers.spectateGame) {
        Handlers.spectateGame(SpectateGameSecret);
    }

    // Right now this batches up any requests and sends them all in a burst; I could imagine a world
    // where the implementer would rather sequentially accept/reject each one before the next invite
    // is sent. I left it this way because I could also imagine wanting to process these all and
    // maybe show them in one common dialog and/or start fetching the avatars in parallel, and if
    // not it should be trivial for the implementer to make a queue themselves.
    while (JoinAskQueue.HavePendingSends()) {
        auto req = JoinAskQueue.GetNextSendMessage();
        if (Handlers.joinRequest) {
            DiscordJoinRequest djr{req->userId, req->username, req->avatar};
            Handlers.joinRequest(&djr);
        }
        JoinAskQueue.CommitSend();
    }

    if (!isConnected) {
        // if we are not connected, disconnect message last
        if (wasDisconnected && Handlers.disconnected) {
            Handlers.disconnected(LastDisconnectErrorCode, LastDisconnectErrorMessage);
        }
    }
}
