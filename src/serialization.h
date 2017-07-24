#pragma once

#include <stdint.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/internal/itoa.h"

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

// Commands
struct DiscordRichPresence;
size_t JsonWriteRichPresenceObj(char* dest, size_t maxLen, int nonce, int pid, const DiscordRichPresence* presence);
size_t JsonWriteSubscribeCommand(char* dest, size_t maxLen, int nonce, const char* evtName);

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

template<size_t Size>
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

using MallocAllocator = rapidjson::CrtAllocator;
extern MallocAllocator MallocAllocatorInst;
using PoolAllocator = rapidjson::MemoryPoolAllocator<MallocAllocator>;
using UTF8 = rapidjson::UTF8<char>;
// Writer appears to need about 16 bytes per nested object level (with 64bit size_t)
using StackAllocator = FixedLinearAllocator<2048>;
constexpr size_t WriterNestingLevels = 2048 / (2 * sizeof(size_t));
using JsonWriter = rapidjson::Writer<DirectStringBuffer, UTF8, UTF8, StackAllocator, rapidjson::kWriteNoFlags>;
using JsonDocument = rapidjson::GenericDocument<UTF8, PoolAllocator, FixedLinearAllocator<2048>>;
