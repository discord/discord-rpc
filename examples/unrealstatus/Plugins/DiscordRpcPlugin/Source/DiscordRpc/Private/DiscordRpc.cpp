// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "DiscordRpcPrivatePCH.h"
#include "IPluginManager.h"
#include "ModuleManager.h"

#define LOCTEXT_NAMESPACE "FDiscordRpcModule"

void FDiscordRpcModule::StartupModule()
{
    // Not needed for Linux
    // Should do nothing for static libs
#if defined(DISCORD_DYNAMIC_LIB)
#if !PLATFORM_LINUX
    // Get the base directory of this plugin
    FString BaseDir = IPluginManager::Get().FindPlugin("DiscordRpc")->GetBaseDir();
    const FString SDKDir = FPaths::Combine(*BaseDir, TEXT("Source"), TEXT("ThirdParty"), TEXT("DiscordRpcLib"));

    // Windows
#if PLATFORM_WINDOWS
    const FString LibName = TEXT("discord-rpc");
#if PLATFORM_64BITS
    const FString LibDir = FPaths::Combine(*SDKDir, TEXT("Win64"));
#elif
    const FString LibDir = FPaths::Combine(*SDKDir, TEXT("Win32"));
#endif

    // Mac
#elif PLATFORM_MAC
    const FString LibName = TEXT("libdiscord-rpc");
    const FString LibDir = FPaths::Combine(*SDKDir, TEXT("Mac"));
#endif

    if (!LoadDependency(LibDir, LibName, DiscordRpcLibraryHandle)) {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT(LOCTEXT_NAMESPACE, "Failed to load DiscordRpc plugin. Plug-in will not be functional."));
        FreeDependency(DiscordRpcLibraryHandle);
    }
#endif
#endif
}

void FDiscordRpcModule::ShutdownModule()
{
    // Free the dll handle
#if !PLATFORM_LINUX
#if defined(DISCORD_DYNAMIC_LIB)
    FreeDependency(DiscordRpcLibraryHandle);
#endif
#endif
}

bool FDiscordRpcModule::LoadDependency(const FString& Dir, const FString& Name, void*& Handle)
{
    // .dll, .so, .dylib
    FString Lib = Name + TEXT(".") + FPlatformProcess::GetModuleExtension();

    // Get full path to dependency
    FString Path = Dir.IsEmpty() ? *Lib : FPaths::Combine(*Dir, *Lib);

    // Load lib
    Handle = FPlatformProcess::GetDllHandle(*Path);
    if (Handle == nullptr)
    {
        return false;
    }

    return true;
}

void FDiscordRpcModule::FreeDependency(void*& Handle)
{
    if (Handle != nullptr)
    {
        // Free lib
        FPlatformProcess::FreeDllHandle(Handle);
        Handle = nullptr;
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDiscordRpcModule, DiscordRpc)