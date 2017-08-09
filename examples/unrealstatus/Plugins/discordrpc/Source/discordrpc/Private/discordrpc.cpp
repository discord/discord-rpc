// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "discordrpc.h"
#include "Core.h"
#include "ModuleManager.h"
#include "IPluginManager.h"

#define DISCORD_DYNAMIC_LIB
//#include "../../../../../../../include/discord-rpc.h"
#include "discord-rpc.h"

#define LOCTEXT_NAMESPACE "FdiscordrpcModule"

void FdiscordrpcModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("discordrpc")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/discordrpcLibrary/Win64/discord-rpc.dll"));
#elif PLATFORM_MAC
    LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/discordrpcLibrary/Mac/Release/libdiscord-rpc.dylib"));
#endif // PLATFORM_WINDOWS

	ExampleLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (ExampleLibraryHandle)
	{
        Discord_Initialize("344609418631053312", nullptr, false);
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load example third party library"));
	}
}

void FdiscordrpcModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(ExampleLibraryHandle);
	ExampleLibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FdiscordrpcModule, discordrpc)