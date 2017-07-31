#pragma once
#include <stdint.h>

// clang-format off

#if defined(DISCORD_DYNAMIC_LIB)
#  if defined(_WIN32)
#    if defined(DISCORD_BUILDING_SDK)
#      define DISCORD_EXPORT __declspec(dllexport)
#    else
#      define DISCORD_EXPORT __declspec(dllimport)
#    endif
#  else
#    define DISCORD_EXPORT __attribute__((visibility("default")))
#  endif
#else
#  define DISCORD_EXPORT 
#endif

// clang-format on

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DiscordRichPresence {
    const char* state;   /* max 128 bytes */
    const char* details; /* max 128 bytes */
    int64_t startTimestamp;
    int64_t endTimestamp;
    const char* largeImageKey;  /* max 32 bytes */
    const char* largeImageText; /* max 128 bytes */
    const char* smallImageKey;  /* max 32 bytes */
    const char* smallImageText; /* max 128 bytes */
    const char* partyId;        /* max 128 bytes */
    int partySize;
    int partyMax;
    const char* matchSecret;    /* max 128 bytes */
    const char* joinSecret;     /* max 128 bytes */
    const char* spectateSecret; /* max 128 bytes */
    int8_t instance;
} DiscordRichPresence;

typedef struct DiscordEventHandlers {
    void (*ready)();
    void (*disconnected)(int errorCode, const char* message);
    void (*errored)(int errorCode, const char* message);
    void (*joinGame)(const char* joinSecret);
    void (*spectateGame)(const char* spectateSecret);
} DiscordEventHandlers;

DISCORD_EXPORT void Discord_Initialize(const char* applicationId,
                                       DiscordEventHandlers* handlers,
                                       int autoRegister);
DISCORD_EXPORT void Discord_Shutdown();

/* checks for incoming messages, dispatches callbacks */
DISCORD_EXPORT void Discord_RunCallbacks();

/* If you disable the lib starting its own io thread, you'll need to call this from your own */
#ifdef DISCORD_DISABLE_IO_THREAD
DISCORD_EXPORT void Discord_UpdateConnection();
#endif

DISCORD_EXPORT void Discord_UpdatePresence(const DiscordRichPresence* presence);

#ifdef __cplusplus
} /* extern "C" */
#endif
