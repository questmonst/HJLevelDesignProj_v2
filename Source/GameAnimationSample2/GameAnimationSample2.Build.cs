// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class GameAnimationSample2 : ModuleRules
{
	public GameAnimationSample2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "Characters/Base"),
			Path.Combine(ModuleDirectory, "Characters/NPC"),
			Path.Combine(ModuleDirectory, "Characters/Player"),
			Path.Combine(ModuleDirectory, "Weapons"),
			Path.Combine(ModuleDirectory, "Core"),
			Path.Combine(ModuleDirectory, "Environment"),
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"MotionWarping",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
