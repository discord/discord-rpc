// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class discordrpc : ModuleRules
{
    public discordrpc(ReadOnlyTargetRules Target) : base(Target)
    {
        Definitions.Add("DISCORD_DYNAMIC_LIB=1");

        PublicIncludePaths.AddRange(
            new string[] {
                "DiscordRpc/Public"
            }
            );

        PrivateIncludePaths.AddRange(
            new string[] {
                "DiscordRpc/Private"
            }
            );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Projects"
            }
            );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );

        string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "DiscordRpcLibrary"));
        PublicIncludePaths.Add(Path.Combine(BaseDirectory, "Include"));
    }
}