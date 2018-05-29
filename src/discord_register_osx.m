#include <stdio.h>
#include <sys/stat.h>

#import <AppKit/AppKit.h>

#include "discord_register.h"

static void RegisterCommand(const char* applicationId, const char* command)
{
    // There does not appear to be a way to register arbitrary commands on OSX, so instead we'll save the command
    // to a file in the Discord config path, and when it is needed, Discord can try to load the file there, open
    // the command therein (will pass to js's window.open, so requires a url-like thing)

    // Note: will not work for sandboxed apps
  	NSString *home = NSHomeDirectory();
    if (!home) {
        return;
    }

    NSString *path = [[[[[[home stringByAppendingPathComponent:@"Library"]
                                stringByAppendingPathComponent:@"Application Support"]
                                stringByAppendingPathComponent:@"discord"]
                                stringByAppendingPathComponent:@"games"]
                                stringByAppendingPathComponent:[NSString stringWithUTF8String:applicationId]]
                                stringByAppendingPathExtension:@"json"];
    [[NSFileManager defaultManager] createDirectoryAtPath:[path stringByDeletingLastPathComponent] withIntermediateDirectories:YES attributes:nil error:nil];

    NSString *jsonBuffer = [NSString stringWithFormat:@"{\"command\": \"%s\"}", command];
    [jsonBuffer writeToFile:path atomically:NO encoding:NSUTF8StringEncoding error:nil];
}

static void RegisterURL(const char* applicationId)
{
    NSString *applicationPath = NSProcessInfo.processInfo.arguments[0];
    const char *cStr = applicationPath.UTF8String;
    RegisterCommand(applicationId, cStr);
}

void Discord_Register(const char* applicationId, const char* command)
{
    if (command) {
        RegisterCommand(applicationId, command);
    }
    else {
        // raii lite
        @autoreleasepool {
            RegisterURL(applicationId);
        }
    }
}

void Discord_RegisterSteamGame(const char* applicationId, const char* steamId)
{
    char command[256];
    snprintf(command, 256, "steam://rungameid/%s", steamId);
    Discord_Register(applicationId, command);
}
