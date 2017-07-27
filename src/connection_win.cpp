#include "connection.h"

#define WIN32_LEAN_AND_MEAN
#define NOMCX
#define NOSERVICE
#define NOIME
#include <windows.h>

int GetProcessId()
{
    return ::GetCurrentProcessId();
}

struct BaseConnectionWin : public BaseConnection {
    HANDLE pipe{INVALID_HANDLE_VALUE};
};

static BaseConnectionWin Connection;

/*static*/ BaseConnection* BaseConnection::Create()
{
    return &Connection;
}

/*static*/ void BaseConnection::Destroy(BaseConnection*& c)
{
    auto self = reinterpret_cast<BaseConnectionWin*>(c);
    self->Close();
    c = nullptr;
}

bool BaseConnection::Open()
{
    wchar_t pipeName[]{L"\\\\?\\pipe\\discord-ipc-0"};
    const size_t pipeDigit = sizeof(pipeName) / sizeof(wchar_t) - 2;
    pipeName[pipeDigit] = L'0';
    auto self = reinterpret_cast<BaseConnectionWin*>(this);
    for (;;) {
        self->pipe = ::CreateFileW(
          pipeName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (self->pipe != INVALID_HANDLE_VALUE) {
            return true;
        }

        auto lastError = GetLastError();
        if (lastError == ERROR_FILE_NOT_FOUND) {
            if (pipeName[pipeDigit] < L'9') {
                pipeName[pipeDigit]++;
                continue;
            }
        }
        else if (lastError == ERROR_PIPE_BUSY) {
            if (!WaitNamedPipeW(pipeName, 10000)) {
                return false;
            }
            continue;
        }
        return false;
    }
}

bool BaseConnection::Close()
{
    auto self = reinterpret_cast<BaseConnectionWin*>(this);
    ::CloseHandle(self->pipe);
    self->pipe = INVALID_HANDLE_VALUE;
    return true;
}

bool BaseConnection::Write(const void* data, size_t length)
{
    auto self = reinterpret_cast<BaseConnectionWin*>(this);
    return ::WriteFile(self->pipe, data, length, nullptr, nullptr) == TRUE;
}

bool BaseConnection::Read(void* data, size_t length)
{
    auto self = reinterpret_cast<BaseConnectionWin*>(this);
    DWORD bytesAvailable = 0;
    if (::PeekNamedPipe(self->pipe, nullptr, 0, nullptr, &bytesAvailable, nullptr)) {
        if (bytesAvailable >= length) {
            if (::ReadFile(self->pipe, data, length, nullptr, nullptr) == TRUE) {
                return true;
            }
        }
    }
    return false;
}
