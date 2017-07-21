/*
    This is a simple example in C of using the rich presence API asyncronously.
*/

#define _CRT_SECURE_NO_WARNINGS /* thanks Microsoft */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "discord-rpc.h"

static const char* APPLICATION_ID = "338030514596216832";
static int FrustrationLevel = 0;

static void updateDiscordPresence() {
    char buffer[256];
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    discordPresence.state = "West of House";
    sprintf(buffer, "Frustration level: %d", FrustrationLevel);
    discordPresence.details = buffer;
    Discord_UpdatePresence(&discordPresence);
}

static void handleDiscordReady() {
    printf("\nDiscord: ready\n");
}

static void handleDiscordDisconnected(int errcode, const char* message) {
    printf("\nDiscord: disconnected (%d: %s)\n", errcode, message);
}

static void handleDiscordPresenceRequested() {
    printf("\nDiscord: requests presence\n");
    updateDiscordPresence();
}

static int prompt(char* line, size_t size) {
    int res;
    char* nl;
    printf("\n> ");
    fflush(stdout);
    res = fgets(line, size, stdin) ? 1 : 0;
    line[size - 1] = 0;
    nl = strchr(line, '\n');
    if (nl) {
        *nl = 0;
    }
    return res;
}

static void gameLoop() {
    char line[512];
    char* space;

    printf("You are standing in an open field west of a white house.\n");
    while (prompt(line, sizeof(line))) {
        if (time(NULL) & 1) {
            printf("I don't understand that.\n");
        } else {
            space = strchr(line, ' ');
            if (space) {
                *space = 0;
            }
            printf("I don't know the word \"%s\".\n", line);
        }

        ++FrustrationLevel;
        
        updateDiscordPresence();
        
#ifdef DISCORD_DISABLE_IO_THREAD
        Discord_UpdateConnection();
#endif
        Discord_RunCallbacks();
    }
}

int main() {
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.presenceRequested = handleDiscordPresenceRequested;
    Discord_Initialize(APPLICATION_ID, &handlers);

    gameLoop();
    
    Discord_Shutdown();
    return 0;
}

