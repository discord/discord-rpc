#include <stdio.h>

#include "discord-rpc.h"

static const char* APPLICATION_ID = "12345678910";

void updateDiscordPresence() {
    DiscordRichPresence myPresence{};
    myPresence.name = "My Awesome Game";
    myPresence.gameState = "In Game";
    myPresence.gameMode = "Summoner's Rift";
    myPresence.gameModifier = "Ranked";
    myPresence.choice = "Aatrox";
    myPresence.flavorImageKey = "FLAVOR_SUMMONERS_RIFT";
    myPresence.choiceImageKey = "PORTRAIT_AATROX";

    Discord_UpdatePresence(&myPresence);    
}

void handleDiscordReady() {
    printf("discord ready\n");
}

void handleDiscordDisconnected() {
    printf("discord disconnected\n");
}

void handleDiscordWantsPresence() {
    printf("discord requests presence\n");
    updateDiscordPresence();
}

void gameLoop() {
    char line[512];
    while (fgets(line, 512, stdin)) {
        line[511] = 0;
        printf("line: %s\n", line);
    }
}

int main() {
    printf("Starting game client\n");

    DiscordEventHandlers handlers{};

    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.wantsPresence = handleDiscordWantsPresence;

    Discord_Initialize(APPLICATION_ID, handlers);

    gameLoop();

    Discord_Shutdown();

    return 0;
}

