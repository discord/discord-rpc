// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class discordrpcLibrary : ModuleRules
{
	public discordrpcLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Include"));
			PublicLibraryPaths.Add(Path.Combine(ModuleDirectory, "x64", "Release"));
			PublicAdditionalLibraries.Add("discord-rpc.lib");

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("discord-rpc.dll");
		}
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "libdiscord-rpc.dylib"));
        }
	}
}
