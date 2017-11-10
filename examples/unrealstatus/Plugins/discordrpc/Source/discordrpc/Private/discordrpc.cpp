// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "discordrpc.h"
#include "Core.h"
#include "IPluginManager.h"
#include "ModuleManager.h"

#define LOCTEXT_NAMESPACE "FdiscordrpcModule"

void FdiscordrpcModule::StartupModule()
{
    // This code will execute after your module is loaded into memory; the exact timing is specified
    // in the .uplugin file per-module

    // Get the base directory of this plugin
    FString BaseDir = IPluginManager::Get().FindPlugin("discordrpc")->GetBaseDir();

    // Add on the relative location of the third party dll and load it
    FString LibraryPath;
#if PLATFORM_WINDOWS
    LibraryPath = FPaths::Combine(
      *BaseDir, TEXT("Binaries/ThirdParty/discordrpcLibrary/Win64/discord-rpc.dll"));
#elif PLATFORM_MAC
    LibraryPath = FPaths::Combine(
      *BaseDir, TEXT("Source/ThirdParty/discordrpcLibrary/Mac/Release/libdiscord-rpc.dylib"));
#endif // PLATFORM_WINDOWS

    DiscordLibraryHandle =
      !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

    if (!DiscordLibraryHandle) {
        FMessageDialog::Open(
          EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load discord-rpc library"));
    }
}

void FdiscordrpcModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that
    // support dynamic reloading,
    // we call this function before unloading the module.

    // Free the dll handle
    FPlatformProcess::FreeDllHandle(DiscordLibraryHandle);
    DiscordLibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FdiscordrpcModule, discordrpc)