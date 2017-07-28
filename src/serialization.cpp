#include "serialization.h"
#include "connection.h"
#include "discord-rpc.h"

MallocAllocator MallocAllocatorInst;

// it's ever so slightly faster to not have to strlen the key
template <typename T>
void WriteKey(JsonWriter& w, T& k)
{
    w.Key(k, sizeof(T) - 1);
}

template <typename T>
void WriteOptionalString(JsonWriter& w, T& k, const char* value)
{
    if (value) {
        w.Key(k, sizeof(T) - 1);
        w.String(value);
    }
}

void JsonWriteNonce(JsonWriter& writer, int nonce)
{
    WriteKey(writer, "nonce");
    char nonceBuffer[32]{};
    rapidjson::internal::i32toa(nonce, nonceBuffer);
    writer.String(nonceBuffer);
}

void JsonWriteCommandStart(JsonWriter& writer, int nonce, const char* cmd)
{
    writer.StartObject();

    JsonWriteNonce(writer, nonce);

    WriteKey(writer, "cmd");
    writer.String(cmd);

    WriteKey(writer, "args");
    writer.StartObject();
}

void JsonWriteCommandEnd(JsonWriter& writer)
{
    writer.EndObject(); // args
    writer.EndObject(); // top level
}

size_t JsonWriteRichPresenceObj(char* dest,
                                size_t maxLen,
                                int nonce,
                                int pid,
                                const DiscordRichPresence* presence)
{
    DirectStringBuffer sb(dest, maxLen);
    StackAllocator wa;
    JsonWriter writer(sb, &wa, WriterNestingLevels);

    JsonWriteCommandStart(writer, nonce, "SET_ACTIVITY");

    WriteKey(writer, "pid");
    writer.Int(pid);

    WriteKey(writer, "activity");
    writer.StartObject();

    WriteOptionalString(writer, "state", presence->state);
    WriteOptionalString(writer, "details", presence->details);

    if (presence->startTimestamp || presence->endTimestamp) {
        WriteKey(writer, "timestamps");
        writer.StartObject();

        if (presence->startTimestamp) {
            WriteKey(writer, "start");
            writer.Int64(presence->startTimestamp);
        }

        if (presence->endTimestamp) {
            WriteKey(writer, "end");
            writer.Int64(presence->endTimestamp);
        }

        writer.EndObject();
    }

    if (presence->largeImageKey || presence->largeImageText || presence->smallImageKey ||
        presence->smallImageText) {
        WriteKey(writer, "assets");
        writer.StartObject();

        WriteOptionalString(writer, "large_image", presence->largeImageKey);
        WriteOptionalString(writer, "large_text", presence->largeImageText);
        WriteOptionalString(writer, "small_image", presence->smallImageKey);
        WriteOptionalString(writer, "small_text", presence->smallImageText);

        writer.EndObject();
    }

    if (presence->partyId || presence->partySize || presence->partyMax) {
        WriteKey(writer, "party");
        writer.StartObject();

        WriteOptionalString(writer, "id", presence->partyId);
        if (presence->partySize) {
            WriteKey(writer, "size");
            writer.StartArray();

            writer.Int(presence->partySize);
            if (0 < presence->partyMax) {
                writer.Int(presence->partyMax);
            }

            writer.EndArray();
        }

        writer.EndObject();
    }

    if (presence->matchSecret || presence->joinSecret || presence->spectateSecret) {
        WriteKey(writer, "secrets");
        writer.StartObject();

        WriteOptionalString(writer, "match", presence->matchSecret);
        WriteOptionalString(writer, "join", presence->joinSecret);
        WriteOptionalString(writer, "spectate", presence->spectateSecret);

        writer.EndObject();
    }

    writer.Key("instance");
    writer.Bool(presence->instance != 0);

    writer.EndObject(); // activity

    JsonWriteCommandEnd(writer);

    return sb.GetSize();
}

size_t JsonWriteHandshakeObj(char* dest, size_t maxLen, int version, const char* applicationId)
{
    DirectStringBuffer sb(dest, maxLen);
    StackAllocator wa;
    JsonWriter writer(sb, &wa, WriterNestingLevels);

    writer.StartObject();
    WriteKey(writer, "v");
    writer.Int(version);
    WriteKey(writer, "client_id");
    writer.String(applicationId);
    writer.EndObject();

    return sb.GetSize();
}

size_t JsonWriteSubscribeCommand(char* dest, size_t maxLen, int nonce, const char* evtName)
{
    DirectStringBuffer sb(dest, maxLen);
    StackAllocator wa;
    JsonWriter writer(sb, &wa, WriterNestingLevels);

    writer.StartObject();

    JsonWriteNonce(writer, nonce);

    WriteKey(writer, "cmd");
    writer.String("SUBSCRIBE");

    WriteKey(writer, "evt");
    writer.String(evtName);

    writer.EndObject();

    return sb.GetSize();
}
