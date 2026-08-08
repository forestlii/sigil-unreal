// Copyright (c) 2026 Likeon. All Rights Reserved.

using UnrealBuildTool;

public class SigilInteraction : ModuleRules
{
	public SigilInteraction(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core", "SmartObjectsModule"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"NetCore",
				"Engine",
				"Slate",
				"SlateCore",
				"GameplayTags",
				"UMG",
				"TargetingSystem",
				"GameplayTasks",
				"GameplayAbilities",
				"GameplayBehaviorsModule",
				"SmartObjectsModule",
				"GameplayBehaviorSmartObjectsModule"
			}
		);
	}
}