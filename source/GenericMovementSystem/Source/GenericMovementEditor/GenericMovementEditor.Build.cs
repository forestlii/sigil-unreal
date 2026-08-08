// Copyright 2024 RedMoonGames All Rights Reserved.

using UnrealBuildTool;

public class GenericMovementEditor : ModuleRules
{
	public GenericMovementEditor(ReadOnlyTargetRules Target) : base(Target)
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
				"GenericMovementSystem",
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