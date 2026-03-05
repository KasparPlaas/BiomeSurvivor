// Copyright Biome Survivor. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BiomeSurvivorTarget : TargetRules
{
	public BiomeSurvivorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;

		ExtraModuleNames.AddRange(new string[]
		{
			"BiomeSurvivor"
		});
	}
}
