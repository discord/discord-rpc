// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class discordrpcLibrary : ModuleRules
{
    public discordrpcLibrary(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;
        Definitions.Add("DISCORD_DYNAMIC_LIB=1");

        string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "discordrpc"));

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string lib = Path.Combine(BaseDirectory, "Win64");

            // Include headers
            PublicIncludePaths.Add(Path.Combine(BaseDirectory, "Include"));

            // Add the import library
            PublicLibraryPaths.Add(lib);
            PublicAdditionalLibraries.Add(Path.Combine(lib, "discord-rpc.lib"));

            // Dynamic
            RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(lib, "discord-rpc.dll")));
            PublicDelayLoadDLLs.Add("discord-rpc.dll");
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string lib = Path.Combine(BaseDirectory, "Linux", "x86_64-unknown-linux-gnu");

            // Include headers
            PublicIncludePaths.Add(Path.Combine(BaseDirectory, "Include"));

            // Add the import library
            PublicLibraryPaths.Add(lib);
            PublicAdditionalLibraries.Add(Path.Combine(lib, "libdiscord-rpc.so"));
            RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(lib, "libdiscord-rpc.so")));
        }
    }
}