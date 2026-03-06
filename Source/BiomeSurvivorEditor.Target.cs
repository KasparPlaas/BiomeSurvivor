// Copyright Biome Survivor. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BiomeSurvivorEditorTarget : TargetRules
{
	public BiomeSurvivorEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;

		ExtraModuleNames.AddRange(new string[]
		{
			"BiomeSurvivor"
		});
	}
}
