// Copyright Biome Survivor. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BiomeSurvivorServerTarget : TargetRules
{
	public BiomeSurvivorServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;

		ExtraModuleNames.AddRange(new string[]
		{
			"BiomeSurvivor"
		});
	}
}
