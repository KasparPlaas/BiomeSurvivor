// Copyright Biome Survivor. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BiomeSurvivorEditorTarget : TargetRules
{
	public BiomeSurvivorEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;

		ExtraModuleNames.AddRange(new string[]
		{
			"BiomeSurvivor"
		});
	}
}
