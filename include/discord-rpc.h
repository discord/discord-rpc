#pragma once
#include <stdint.h>

//#define DISCORD_DISABLE_IO_THREAD

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct {
    const char* state;
    const char* details;
    int64_t startTimestamp;
    int64_t endTimestamp;
    const char* largeImageKey;
    const char* largeImageText;
    const char* smallImageKey;
    const char* smallImageText;
    const char* partyId;
    int partySize;
    int partyMax;
    const char* matchSecret;
    const char* joinSecret;
    const char* spectateSecret;
    int8_t instance;
} DiscordRichPresence;

typedef struct {
    void (*ready)();
    void (*disconnected)(int errorCode, const char* message);
    void (*wantsPresence)();
    void (*joinGame)(const char* joinSecret);
    void (*spectateGame)(const char* spectateSecret);
} DiscordEventHandlers;

void Discord_Initialize(const char* applicationId, DiscordEventHandlers* handlers);
void Discord_Shutdown();
void Discord_UpdatePresence(const DiscordRichPresence* presence);

/* checks for incoming messages, dispatches callbacks */
void Discord_RunCallbacks();

/* If you disable the lib starting its own io thread, you'll need to call this from your own */
#ifdef DISCORD_DISABLE_IO_THREAD
void Discord_UpdateConnection();
#endif

#ifdef  __cplusplus
} /* extern "C" */
#endif
