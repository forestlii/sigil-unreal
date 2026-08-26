// Copyright (c) 2026 Likeon. All Rights Reserved.

using UnrealBuildTool;

public class SigilNarrativeEditor : ModuleRules
{
	public SigilNarrativeEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Engine",
				"SigilNarrative",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"AssetDefinition",
				"PropertyEditor"
			}
		);
	}
}
