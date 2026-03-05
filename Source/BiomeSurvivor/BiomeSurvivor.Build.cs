// Copyright Biome Survivor. All Rights Reserved.

using UnrealBuildTool;

public class BiomeSurvivor : ModuleRules
{
	public BiomeSurvivor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"AIModule",
			"NavigationSystem",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"Niagara",
			"PhysicsCore",
			"NetCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Sockets",
			"Networking"
		});

		// Enable IWYU (Include What You Use) for faster compilation
		bEnforceIWYU = true;
	}
}
