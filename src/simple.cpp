#include <stdio.h>

#include "discord-rpc.h"

static const char* APPLICATION_ID = "12345678910";

void updateDiscordPresence() {
    DiscordRichPresence discordPresence{};

    discordPresence.state = "In a Group";
    discordPresence.details = "Competitive\nIn a \"Match\"";
    discordPresence.endTimestamp = time(nullptr) + ((60 * 5) + 23);
    discordPresence.partyId = "12345";
    discordPresence.partySize = 3;
    discordPresence.partyMax = 6;
    discordPresence.matchSecret = "4b2fdce12f639de8bfa7e3591b71a0d679d7c93f";
    discordPresence.spectateSecret = "e7eb30d2ee025ed05c71ea495f770b76454ee4e0";
    discordPresence.instance = true;

    Discord_UpdatePresence(&discordPresence);
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
    printf("> ");
    fflush(stdout);
    while (fgets(line, 512, stdin)) {
        line[511] = 0;
        printf("I don't understand that.\n> ");
        fflush(stdout);

        updateDiscordPresence();
    }
}

int main() {
    printf("Starting game client\n");

    DiscordEventHandlers handlers{};

    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.wantsPresence = handleDiscordWantsPresence;

    Discord_Initialize(APPLICATION_ID, &handlers);

    gameLoop();

    Discord_Shutdown();

    return 0;
}

