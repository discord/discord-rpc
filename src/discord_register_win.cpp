#include "discord-rpc.h"
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#define NOMCX
#define NOSERVICE
#define NOIME
#include <windows.h>
#include <Psapi.h>
#include <Strsafe.h>
#pragma comment(lib, "Psapi.lib")

void Discord_RegisterW(const wchar_t* applicationId, const wchar_t* command)
{
    // https://msdn.microsoft.com/en-us/library/aa767914(v=vs.85).aspx
    // we want to register games so we can run them as discord-<appid>://
    // Update the HKEY_CURRENT_USER, because it doesn't seem to require special permissions.

    wchar_t exeFilePath[MAX_PATH];
    DWORD exeLen = GetModuleFileNameExW(GetCurrentProcess(), nullptr, exeFilePath, MAX_PATH);
    wchar_t openCommand[1024];

    if (command && command[0]) {
        StringCbPrintfW(openCommand, sizeof(openCommand), L"%s", command);
    }
    else {
        lstrcpyW(openCommand, exeFilePath);
    }

    wchar_t protocolName[64];
    StringCbPrintfW(protocolName, sizeof(protocolName), L"discord-%s", applicationId);
    wchar_t protocolDescription[128];
    StringCbPrintfW(
      protocolDescription, sizeof(protocolDescription), L"URL:Run game %s protocol", applicationId);
    wchar_t urlProtocol = 0;

    wchar_t keyName[256];
    StringCbPrintfW(keyName, sizeof(keyName), L"Software\\Classes\\%s", protocolName);
    HKEY key;
    auto status =
      RegCreateKeyExW(HKEY_CURRENT_USER, keyName, 0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr);
    if (status != ERROR_SUCCESS) {
        fprintf(stderr, "Error creating key\n");
        return;
    }
    DWORD len;
    LSTATUS result;
    len = (DWORD)lstrlenW(protocolDescription) + 1;
    result =
      RegSetKeyValueW(key, nullptr, nullptr, REG_SZ, protocolDescription, len * sizeof(wchar_t));
    if (FAILED(result)) {
        fprintf(stderr, "Error writing description\n");
    }

    len = (DWORD)lstrlenW(protocolDescription) + 1;
    result = RegSetKeyValueW(key, nullptr, L"URL Protocol", REG_SZ, &urlProtocol, sizeof(wchar_t));
    if (FAILED(result)) {
        fprintf(stderr, "Error writing description\n");
    }

    result = RegSetKeyValueW(
      key, L"DefaultIcon", nullptr, REG_SZ, exeFilePath, (exeLen + 1) * sizeof(wchar_t));
    if (FAILED(result)) {
        fprintf(stderr, "Error writing icon\n");
    }

    len = (DWORD)lstrlenW(openCommand) + 1;
    result = RegSetKeyValueW(
      key, L"shell\\open\\command", nullptr, REG_SZ, openCommand, len * sizeof(wchar_t));
    if (FAILED(result)) {
        fprintf(stderr, "Error writing command\n");
    }
    RegCloseKey(key);
}

extern "C" void Discord_Register(const char* applicationId, const char* command)
{
    wchar_t appId[32];
    MultiByteToWideChar(CP_UTF8, 0, applicationId, -1, appId, 32);

    wchar_t openCommand[1024];
    const wchar_t* wcommand = nullptr;
    if (command && command[0]) {
        const auto commandBufferLen = sizeof(openCommand) / sizeof(*openCommand);
        MultiByteToWideChar(CP_UTF8, 0, command, -1, openCommand, commandBufferLen);
        wcommand = openCommand;
    }

    Discord_RegisterW(appId, wcommand);
}

extern "C" void Discord_RegisterSteamGame(const char* applicationId, const char* steamId)
{
    wchar_t appId[32];
    MultiByteToWideChar(CP_UTF8, 0, applicationId, -1, appId, 32);

    wchar_t wSteamId[32];
    MultiByteToWideChar(CP_UTF8, 0, steamId, -1, wSteamId, 32);

    HKEY key;
    auto status = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", 0, KEY_READ, &key);
    if (status != ERROR_SUCCESS) {
        fprintf(stderr, "Error opening Steam key\n");
        return;
    }

    wchar_t steamPath[MAX_PATH];
    DWORD pathBytes = sizeof(steamPath);
    status = RegQueryValueExW(key, L"SteamExe", nullptr, nullptr, (BYTE*)steamPath, &pathBytes);
    RegCloseKey(key);
    if (status != ERROR_SUCCESS || pathBytes < 1) {
        fprintf(stderr, "Error reading SteamExe key\n");
        return;
    }

    DWORD pathChars = pathBytes / sizeof(wchar_t);
    for (DWORD i = 0; i < pathChars; ++i) {
        if (steamPath[i] == L'/') {
            steamPath[i] = L'\\';
        }
    }

    wchar_t command[1024];
    StringCbPrintfW(command, sizeof(command), L"\"%s\" steam://run/%s", steamPath, wSteamId);

    Discord_RegisterW(appId, command);
}
