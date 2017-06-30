#pragma once
#include <stdint.h>

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
    void (*disconnected)();
    void (*wantsPresence)();
    void (*joinGame)(const char* joinSecret);
    void (*spectateGame)(const char* spectateSecret);
} DiscordEventHandlers;

void Discord_Initialize(const char* applicationId, DiscordEventHandlers* handlers);
void Discord_Shutdown();
void Discord_UpdatePresence(const DiscordRichPresence* presence);

/* later

struct DiscordChannelEventHandlers {
    void (*messageCreate)(const DiscordMessage* message);
    void (*messageUpdate)(const DiscordMessage* message);
    void (*messageDelete)(const DiscordMessage* message);
    void (*voiceStateCreate)(const DiscordVoiceState* state);
    void (*voiceStateDelete)(const DiscordVoiceState* state);
    void (*speakingStart)(const DiscordSpeakingState* state);
    void (*speakingStop)(const DiscordSpeakingState* state);    
};
// Call this to subscribe to events in a specific voice or text channel
void Discord_Subscribe(const char* channelId, const DiscordChannelEventHandlers* handlers);

// TBD RPC Requests
void Discord_Authenticate();
void Discord_Authorize();
void Discord_GetGuilds();
void Discord_GetChannels();
void Discord_GetChannel();
void Discord_SelectVoiceChannel();
void Discord_SelectTextChannel();
void Discord_SendMessage();

*/

#ifdef  __cplusplus
} /* extern "C" */
#endif
