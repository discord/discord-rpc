#pragma once

/*
    This is as simple of a json writing thing as possible; does not try to keep you
    from overflowing buffer, so make sure you have room.
*/

// if only there was a standard library function for this
inline size_t StringCopy(char* dest, const char* src, size_t maxBytes = UINT32_MAX) {
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

inline void JsonWriteEscapedString(char*& dest, const char* src)
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

inline void JsonWritePropName(char*& dest, const char* name)
{
    *dest++ = '"';
    dest += StringCopy(dest, name);
    *dest++ = '"';
    *dest++ = ':';
    *dest++ = ' ';
}

inline void JsonWritePropSep(char*& dest)
{
    *dest++ = ',';
    *dest++ = ' ';
}

inline void JsonWriteStringProp(char*& dest, const char* name, const char* value)
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

inline void JsonWriteBoolProp(char*& dest, const char* name, bool value)
{
    JsonWritePropName(dest, name);
    dest += StringCopy(dest, value ? "true" : "false");
    JsonWritePropSep(dest);
}

inline void JsonWriteRichPresenceObj(char*& dest, const DiscordRichPresence* presence)
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
}
