#include <stdio.h>
#include <sys/stat.h>

#import <AppKit/AppKit.h>

#include "discord_register.h"

static bool Mkdir(const char* path)
{
    int result = mkdir(path, 0755);
    if (result == 0) {
        return true;
    }
    if (errno == EEXIST) {
        return true;
    }
    return false;
}

static void RegisterCommand(const char* applicationId, const char* command)
{
    // There does not appear to be a way to register arbitrary commands on OSX, so instead we'll save the command
    // to a file in the Discord config path, and when it is needed, Discord can try to load the file there, open
    // the command therein (will pass to js's window.open, so requires a url-like thing)

    const char* home = getenv("HOME");
    if (!home) {
        return;
    }

    char path[2048];
    sprintf(path, "%s/Library/Application Support/discord", home);
    Mkdir(path);
    strcat(path, "/games");
    Mkdir(path);
    strcat(path, "/");
    strcat(path, applicationId);
    strcat(path, ".json");

    FILE* f = fopen(path, "w");
    if (f) {
        char jsonBuffer[2048];
        int len = snprintf(jsonBuffer, sizeof(jsonBuffer), "{\"command\": \"%s\"}", command);
        fwrite(jsonBuffer, (size_t)len, 1, f);
        fclose(f);
    }
}

static void RegisterURL(const char* applicationId)
{
    char url[256];
    snprintf(url, sizeof(url), "discord-%s", applicationId);
    CFStringRef cfURL = CFStringCreateWithCString(NULL, url, kCFStringEncodingUTF8);

    NSString* myBundleId = [[NSBundle mainBundle] bundleIdentifier];
    if (!myBundleId) {
        fprintf(stderr, "No bundle id found\n");
        return;
    }

    NSURL* myURL = [[NSBundle mainBundle] bundleURL];
    if (!myURL) {
        fprintf(stderr, "No bundle url found\n");
        return;
    }

    OSStatus status = LSSetDefaultHandlerForURLScheme(cfURL, (__bridge CFStringRef)myBundleId);
    if (status != noErr) {
        fprintf(stderr, "Error in LSSetDefaultHandlerForURLScheme: %d\n", (int)status);
        return;
    }

    status = LSRegisterURL((__bridge CFURLRef)myURL, true);
    if (status != noErr) {
        fprintf(stderr, "Error in LSRegisterURL: %d\n", (int)status);
    }
}

void Discord_Register(const char* applicationId, const char* command)
{
    if (command) {
        RegisterCommand(applicationId, command);
    }
    else {
        // raii lite
        void* pool = [[NSAutoreleasePool alloc] init];
        RegisterURL(applicationId);
        [(id)pool drain];
    }
}

void Discord_RegisterSteamGame(const char* applicationId, const char* steamId)
{
    char command[256];
    sprintf(command, "steam://run/%s", steamId);
    Discord_Register(applicationId, command);
}
