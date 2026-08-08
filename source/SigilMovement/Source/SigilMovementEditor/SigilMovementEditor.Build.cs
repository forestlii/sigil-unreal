// Copyright (c) 2026 Likeon. All Rights Reserved.

using UnrealBuildTool;

public class SigilMovementEditor : ModuleRules
{
	public SigilMovementEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		bEnableNonInlinedGenCppWarnings = true;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"SlateCore",
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"SigilMovement",
				"AnimationModifiers", 
				"AnimationBlueprintLibrary",
			}
		);

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new[]
			{
				"UnrealEd",
				"AnimGraph", 
				"AnimGraphRuntime", 
				"BlueprintGraph", 
				"ToolMenus",
				"Blutility"
			});
		}
	}
}