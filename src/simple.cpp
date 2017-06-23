
static const char* YOUR_APPLICATION_ID = "23984729347234";

void updateDiscordPresence() {
    DiscordRichPresence myPresence = {0};
    myPresence.name = "League of Legends";
    myPresence.gameState = "In Game";
    myPresence.gameMode = "Summoner's Rift";
    myPresence.gameModifier = "Ranked";
    myPresence.choice = "Aatrox";
    myPresence.flavorImageKey = "FLAVOR_SUMMONERS_RIFT";
    myPresence.choiceImageKey = "PORTRAIT_AATROX";

    myPresence.partyId = GameEngine_GetMultiplayerPartyId();
    myPresence.partySize = GameEngine_GetCurrentPartyCount();
    myPresence.partyCapacity = PARTY_CAPACITY;
    
    myPresence.context = GameEngine_GetPartyAndMatchSecret();
    myPresence.isInstance = true;

    myPresence.joinSecret = GameEngine_GetPartyAndMatchSecret();
    myPresence.spectateSecret = GameEngine_GetUserIdSecret();

    Discord_UpdatePresence(&myPresence);    
}

void handleDiscordDisconnected() {
    // oh noes
}

void handleDiscordReady() {
    updateDiscordPresence();
}

void handleDiscordWantsPresence() {
    updateDiscordPresence();
}

void handleDiscordJoinGame(const char* joinSecret) {
    GameEngine_JoinParty(joinSecret);
}
    
void handleDiscordSpectateGame(const char* spectateSecret) {
    GameEngine_SpectateGame(spectateSecret);
}

int main() {
    DiscordEventHandlers handlers = {0};
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.wantsPresence = handleDiscordWantsPresence;
    handlers.joinGame = handleDiscordJoinGame;
    handlers.spectateGame = handleDiscordSpectateGame;

    Discord_Initialize(YOUR_APPLICATION_ID, handlers);
}

