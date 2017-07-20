#include "connection.h"
#include "discord-rpc.h"

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

// I want to use as few allocations as I can get away with, and to do that with RapidJson, you need to supply some of
// your own allocators for stuff rather than use the defaults

class LinearAllocator {
public:
    char* buffer_;
    char* end_;
    LinearAllocator() {
        assert(0); // needed for some default case in rapidjson, should not use
    }
    LinearAllocator(char* buffer, size_t size) : buffer_(buffer), end_(buffer + size) {}
    static const bool kNeedFree = false;
    void* Malloc(size_t size)
    {
        char* res = buffer_;
        buffer_ += size;
        if (buffer_ > end_) {
            buffer_ = res;
            return nullptr;
        }
        return res;
    }
    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize)
    {
        if (newSize == 0) {
            return nullptr;
        }
        // allocate how much you need in the first place
        assert(!originalPtr && !originalSize);
        return Malloc(newSize);
    }
    static void Free(void* ptr) { /* shrug */ }
};

template<int Size>
class FixedLinearAllocator : public LinearAllocator {
public:
    char fixedBuffer_[Size];
    FixedLinearAllocator() : LinearAllocator(fixedBuffer_, Size) {}
    static const bool kNeedFree = false;
};

// wonder why this isn't a thing already, maybe I missed it
class DirectStringBuffer {
public:
    typedef typename char Ch;
    char* buffer_;
    char* end_;
    char* current_;

    DirectStringBuffer(char* buffer, size_t maxLen)
        : buffer_(buffer)
        , end_(buffer + maxLen)
        , current_(buffer)
    {}

    void Put(char c)
    {
        if (current_ < end_) {
            *current_++ = c;
        }
    }
    void Flush() {}
    size_t GetSize() const
    {
        return current_ - buffer_;
    }
};

using Encoding = rapidjson::UTF8<char>;
// Writer appears to need about 16 bytes per nested object level (with 64bit size_t)
using WriterAllocator = FixedLinearAllocator<2048>;
constexpr size_t WriterNestingLevels = 2048 / 16;
using JsonWriter = rapidjson::Writer<DirectStringBuffer, Encoding, Encoding, WriterAllocator, rapidjson::kWriteNoFlags>;

// it's ever so slightly faster to not have to strlen the key
template<typename T>
void WriteKey(JsonWriter& w, T& k) {
    w.Key(k, sizeof(T) - 1);
}

template<typename T>
void WriteOptionalString(JsonWriter& w, T& k, const char* value) {
    if (value) {
        w.Key(k, sizeof(T) - 1);
        w.String(value);
    }
}

size_t JsonWriteRichPresenceObj(char* dest, size_t maxLen, const DiscordRichPresence* presence)
{
    DirectStringBuffer sb(dest, maxLen);
    WriterAllocator wa;
    JsonWriter writer(sb, &wa, WriterNestingLevels);

    // const args = {pid, activity};
    // this.socket.write(encode(OPCODES.FRAME, { nonce: uuid(), cmd : 'SET_ACTIVITY', args })

    writer.StartObject();

    WriteKey(writer, "args");
    writer.StartObject();

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

    if (presence->largeImageKey || presence->largeImageText || presence->smallImageKey || presence->smallImageText) {
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

    writer.EndObject(); // args

    writer.EndObject(); // top level

    return sb.GetSize();
}

size_t JsonWriteHandshakeObj(char* dest, size_t maxLen, int version, const char* applicationId)
{
    DirectStringBuffer sb(dest, maxLen);
    WriterAllocator wa;
    JsonWriter writer(sb, &wa, WriterNestingLevels);

    writer.StartObject();
    WriteKey(writer, "v");
    writer.Int(version);
    WriteKey(writer, "client_id");
    writer.String(applicationId);
    writer.EndObject();

    return sb.GetSize();
}

