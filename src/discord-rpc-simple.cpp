#include "discord-rpc.h"

#include <stdio.h>

// todo: think about making per-platform versions of this whole file, or wrapping the platform specific parts -- win32 for first version

// I just want the basics
#define WIN32_LEAN_AND_MEAN
#define NOMCX
#define NOSERVICE
#define NOIME
#include <windows.h>

namespace {

const int RpcVersion = 1;

enum class OPCODE : uint32_t {
    HANDSHAKE = 0,
    FRAME = 1,
    CLOSE = 2,
};

struct RpcMessageFrame {
    OPCODE opcode;
    uint32_t length;
    char message[64 * 1024 - 8];
};

static const wchar_t* PipeName = L"\\\\?\\pipe\\discord-ipc";
static HANDLE PipeHandle{ INVALID_HANDLE_VALUE };

static char ApplicationId[64]{};
static DiscordEventHandlers Handlers{};
static RpcMessageFrame Frame;

// if only there was a standard library function for this
size_t StringCopy(char* dest, const char* src, size_t maxBytes = UINT32_MAX) {
    if (!dest || !src || !maxBytes) {
        return 0;
    }
    size_t copied;
    for (copied = 1; *src && copied < maxBytes; ++copied) {
        *dest++ = *src++;
    }
    *dest = 0;
    return copied - 1;
}

void JsonWriteEscapedString(char*& dest, const char* src)
{
    for (char c = *src++; c; c = *src++) {
        switch (c) {
        case '\"':
        case '\\':
            *dest++ = '\\';
            *dest++ = c;
            break;
        case '\b':
            *dest++ = '\\';
            *dest++ = 'b';
            break;
        case '\f':
            *dest++ = '\\';
            *dest++ = 'f';
            break;
        case '\n':
            *dest++ = '\\';
            *dest++ = 'n';
            break;
        case '\r':
            *dest++ = '\\';
            *dest++ = 'r';
            break;
        case '\t':
            *dest++ = '\\';
            *dest++ = 't';
            break;
        default:
            *dest++ = c;
            break;
        }
    }
}

template<typename T> void JsonWriteNumber(char*& dest, T number)
{
    if (!number) {
        *dest++ = '0';
        return;
    }
    if (number < 0) {
        *dest++ = '-';
        number = -number;
    }
    char temp[32];
    int place = 0;
    while (number) {
        auto digit = number % 10;
        number = number / 10;
        temp[place++] = '0' + (char)digit;
    }
    for (--place; place >= 0; --place) {
        *dest++ = temp[place];
    }
    *dest = 0;
}

void JsonWritePropName(char*& dest, const char* name)
{
    *dest++ = '"';
    dest += StringCopy(dest, name);
    *dest++ = '"';
    *dest++ = ':';
    *dest++ = ' ';
}

void JsonWritePropSep(char*& dest)
{
    *dest++ = ',';
    *dest++ = ' ';
}

void JsonWriteStringProp(char*& dest, const char* name, const char* value)
{
    JsonWritePropName(dest, name);
    *dest++ = '"';
    JsonWriteEscapedString(dest, value);
    *dest++ = '"';
    JsonWritePropSep(dest);
}

template<typename T>
void JsonWriteNumberAsStringProp(char*& dest, const char* name, T value)
{
    JsonWritePropName(dest, name);
    *dest++ = '"';
    JsonWriteNumber(dest, value);
    *dest++ = '"';
    JsonWritePropSep(dest);
}

template<typename T>
void JsonWriteNumberProp(char*& dest, const char* name, T value)
{
    JsonWritePropName(dest, name);
    JsonWriteNumber(dest, value);
    JsonWritePropSep(dest);
}

void JsonWriteBoolProp(char*& dest, const char* name, bool value)
{
    JsonWritePropName(dest, name);
    dest += StringCopy(dest, value ? "true" : "false");
    JsonWritePropSep(dest);
}

void JsonWriteRichPresenceObj(char*& dest, const DiscordRichPresence* presence)
{
    *dest++ = '{';

    if (presence->state) {
        JsonWriteStringProp(dest, "state", presence->state);
    }

    if (presence->details) {
        JsonWriteStringProp(dest, "details", presence->details);
    }

    if (presence->startTimestamp) {
        JsonWriteNumberAsStringProp(dest, "start_timestamp", presence->startTimestamp);
    }

    if (presence->endTimestamp) {
        JsonWriteNumberAsStringProp(dest, "end_timestamp", presence->endTimestamp);
    }

    if (presence->largeImageKey) {
        JsonWriteStringProp(dest, "large_image_key", presence->largeImageKey);
    }

    if (presence->largeImageText) {
        JsonWriteStringProp(dest, "large_image_text", presence->largeImageText);
    }

    if (presence->smallImageKey) {
        JsonWriteStringProp(dest, "small_image_key", presence->smallImageKey);
    }

    if (presence->smallImageText) {
        JsonWriteStringProp(dest, "small_image_text", presence->smallImageText);
    }

    if (presence->partyId) {
        JsonWriteStringProp(dest, "party_id", presence->partyId);
    }

    if (presence->partyMax) {
        JsonWriteNumberProp(dest, "party_size", presence->partySize);
        JsonWriteNumberProp(dest, "party_max", presence->partyMax);
    }

    if (presence->matchSecret) {
        JsonWriteStringProp(dest, "match_secret", presence->matchSecret);
    }

    if (presence->joinSecret) {
        JsonWriteStringProp(dest, "join_secret", presence->joinSecret);
    }

    if (presence->spectateSecret) {
        JsonWriteStringProp(dest, "spectate_secret", presence->spectateSecret);
    }

    JsonWriteBoolProp(dest, "instance", presence->instance != 0);

    dest -= 1;
    *(dest - 1) = '}';
    *dest = 0;
}

void JsonWriteHandshakeObj(char*& dest, const char* applicationId)
{
    *dest++ = '{';

    JsonWriteNumberProp(dest, "v", RpcVersion);
    JsonWriteStringProp(dest, "client_id", applicationId);

    dest -= 1;
    *(dest - 1) = '}';
    *dest = 0;
}

void ConnectionWrite(const RpcMessageFrame* frame);

void ConnectionOpen()
{
    for (;;) {
        PipeHandle = CreateFileW(PipeName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (PipeHandle != INVALID_HANDLE_VALUE) {
            if (Handlers.ready) {
                Handlers.ready();
            }
            break;
        }

        DWORD err = GetLastError();

        if (err == ERROR_FILE_NOT_FOUND) {
            printf("No server running\n");
            return;
        }

        if (err != ERROR_PIPE_BUSY) {
            printf("Could not open pipe. Error: %d\n", GetLastError());
            return;
        }

        if (!WaitNamedPipeW(PipeName, 10000)) {
            printf("Could not open pipe: 10 second wait timed out.\n");
            return;
        }
    }

    RpcMessageFrame frame;
    frame.opcode = OPCODE::HANDSHAKE;
    char* msg = frame.message;
    JsonWriteHandshakeObj(msg, ApplicationId);
    frame.length = msg - frame.message;
    ConnectionWrite(&frame);
}

void ConnectionClose()
{
    CloseHandle(PipeHandle);
    PipeHandle = INVALID_HANDLE_VALUE;
    if (Handlers.disconnected) {
        Handlers.disconnected(0, "");
    }
}

void ConnectionWrite(const RpcMessageFrame* frame)
{
    if (PipeHandle == INVALID_HANDLE_VALUE) {
        ConnectionOpen();
        if (PipeHandle == INVALID_HANDLE_VALUE) {
            return;
        }
    }
    BOOL success = WriteFile(PipeHandle, frame, 8 + frame->length, nullptr, nullptr);
    if (!success) {
        ConnectionClose();
    }
}

} // anonymous namespace

extern "C" void Discord_Initialize(const char* applicationId, DiscordEventHandlers* handlers)
{
    StringCopy(ApplicationId, applicationId, sizeof(ApplicationId));
    if (handlers) {
        Handlers = *handlers;
    }
    else {
        Handlers = {};
    }

    ConnectionOpen();
}

extern "C" void Discord_Shutdown()
{
    Handlers = {};
    ConnectionClose();
}

extern "C" void Discord_UpdatePresence(const DiscordRichPresence* presence)
{
    Frame.opcode = OPCODE::FRAME;
    char* jsonWrite = Frame.message;
    JsonWriteRichPresenceObj(jsonWrite, presence);
    Frame.length = jsonWrite - Frame.message;
    ConnectionWrite(&Frame);
}
