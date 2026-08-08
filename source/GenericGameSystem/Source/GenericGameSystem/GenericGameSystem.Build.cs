// Copyright 2025 RedMoonGames All Rights Reserved.

using UnrealBuildTool;

public class GenericGameSystem : ModuleRules
{
	public GenericGameSystem(ReadOnlyTargetRules Target) : base(Target)
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