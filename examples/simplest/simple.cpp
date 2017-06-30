
// ug
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "discord-rpc.h"

/*
    This is a C++ (but really mostly C) simple example of just using the rich presence API.
*/

static const char* APPLICATION_ID = "12345678910";
static int FrustrationLevel = 0;

static void updateDiscordPresence() {
    DiscordRichPresence discordPresence{};
    discordPresence.state = "West of House";
    char buffer[256];
    sprintf(buffer, "Frustration level: %d", FrustrationLevel);
    discordPresence.details = buffer;
    Discord_UpdatePresence(&discordPresence);
}

static void handleDiscordReady() {
    printf("Discord: ready\n");
}

static void handleDiscordDisconnected() {
    printf("Discord: disconnected\n");
}

static void handleDiscordWantsPresence() {
    printf("Discord: requests presence\n");
    updateDiscordPresence();
}

static bool prompt(char* line, size_t size) {
    printf("\n> ");
    fflush(stdout);
    bool res = fgets(line, size, stdin) != nullptr;
    line[size - 1] = 0;
    char* nl = strchr(line, '\n');
    if (nl) {
        *nl = 0;
    }
    return res;
}

static void gameLoop() {
    printf("You are standing in an open field west of a white house.\n");
    char line[512];
    while (prompt(line, sizeof(line))) {
        if (time(NULL) & 1) {
            printf("I don't understand that.\n");
        } else {
            char* space = strchr(line, ' ');
            if (space) {
                *space = 0;
            }
            printf("I don't know the word \"%s\".\n", line);
        }

        ++FrustrationLevel;
        
        updateDiscordPresence();
    }
}

int main() {
    DiscordEventHandlers handlers{};
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.wantsPresence = handleDiscordWantsPresence;
    Discord_Initialize(APPLICATION_ID, &handlers);

    gameLoop();
    
    Discord_Shutdown();
    return 0;
}

