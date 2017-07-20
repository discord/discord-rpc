#pragma once

#include <stdint.h>

// if only there was a standard library function for this
template<size_t Len>
inline size_t StringCopy(char (&dest)[Len], const char* src) {
    if (!dest || !src || !Len) {
        return 0;
    }
    size_t copied;
    char* out = dest;
    for (copied = 1; *src && copied < Len; ++copied) {
        *out++ = *src++;
    }
    *out = 0;
    return copied - 1;
}

size_t JsonWriteHandshakeObj(char* dest, size_t maxLen, int version, const char* applicationId);

struct DiscordRichPresence;
size_t JsonWriteRichPresenceObj(char* dest, size_t maxLen, char* nonce, int pid, const DiscordRichPresence* presence);

