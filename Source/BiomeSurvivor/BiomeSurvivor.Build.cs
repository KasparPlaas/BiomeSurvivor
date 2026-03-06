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

		// Ensure subdirectory includes resolve from module root
		PublicIncludePaths.AddRange(new string[] { ModuleDirectory });

		// Enable IWYU (Include What You Use) for faster compilation
		IWYUSupport = IWYUSupport.Full;
	}
}
