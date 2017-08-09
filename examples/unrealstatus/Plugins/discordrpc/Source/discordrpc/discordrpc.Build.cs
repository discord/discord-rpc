// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class discordrpc : ModuleRules
{
	public discordrpc(ReadOnlyTargetRules Target) : base(Target)
	{
        Definitions.Add("DISCORD_DYNAMIC_LIB=1");
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				"discordrpc/Public"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"discordrpc/Private",
                "../../../../../include"
				// ... add other private include paths required here ...
			}
			);

        PublicLibraryPaths.AddRange(
            new string[] {
                System.IO.Path.Combine(ModuleDirectory, "../../Binaries/ThirdParty/discordrpcLibrary/", Target.Platform.ToString()),
			}
            );


        PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Core",
				"discordrpcLibrary",
				"Projects"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
