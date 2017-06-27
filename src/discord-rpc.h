#pragma once

struct DiscordRichPresence {
    uint64_t contextStartTimeUTC; // 0 means unspecified
	uint64_t contextStopTimeUTC; // 0 means unspecified

    ////
    const char* gameState;  // eg. In Game
    const char* gameMode;   // e.g. Summoner's Rift
    const char* gameModifier; // e.g. Ranked
    const char* choice; // e.g. Aatrox
    // or
	const char* partyStatus[4]; // e.g. "In Game", "Summoner's Rift", "Ranked", "Aatrox"
    ////

    const char* largeImageKey; // e.g. The map background
    const char* smallImageKey; // e.g. The character's portrait icon
    
    const char* partyId;
    uint16_t partySize; // e.g. 0 means ignored
    uint16_t partyCapacity; // e.g. 0 means no limit

    // optional
    const char* contextSecret; // Required for the "notify me" feature
    uint8_t isInstance; // Together with context_secret enables the "notify me" feature

    const char* joinSecret; // Enables the "invite to join" feature
    const char* spectateSecret; // Enables the "invite to spectate" feature
};

struct DiscordEventHandlers {
    // required.
    void (*ready)();
    void (*disconnected)();

    // optional for rich presence
    void (*wantsPresence)();
    void (*joinGame)(const char* joinSecret);
    void (*spectateGame)(const char* spectateSecret);
};

struct DiscordChannelEventHandlers {
    void (*messageCreate)(const DiscordMessage* message);
    void (*messageUpdate)(const DiscordMessage* message);
    void (*messageDelete)(const DiscordMessage* message);
    void (*voiceStateCreate)(const DiscordVoiceState* state);
    void (*voiceStateDelete)(const DiscordVoiceState* state);
    void (*speakingStart)(const DiscordSpeakingState* state);
    void (*speakingStop)(const DiscordSpeakingState* state);    
}

// Call this to start up the Discord SDK
void Discord_Initialize(const char* applicationId, DiscordEventHandlers* handlers);

// Call this to subscribe to events in a specific voice or text channel
void Discord_Subscribe(const char* channelId, const DiscordChannelEventHandlers* handlers)

// Call this when you're all done so we can cleanup without timing out.
void Discord_Shutdown();

// Call this whenever any of the data in the payload changes in a material way.
void Discord_UpdatePresence(const DiscordRichPresence* presence);

// TBD RPC Requests
void Discord_Authenticate();
void Discord_Authorize();
void Discord_GetGuilds();
void Discord_GetChannels();
void Discord_GetChannel();
void Discord_SelectVoiceChannel();
void Discord_SelectTextChannel();
void Discord_SendMessage();
