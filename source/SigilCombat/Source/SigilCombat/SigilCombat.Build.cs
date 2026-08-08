// Copyright (c) 2026 Likeon. All Rights Reserved.

using UnrealBuildTool;

public class SigilCombat : ModuleRules
{
	public SigilCombat(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayTags",
				"GameplayTasks",
				"GameplayAbilities",
				"SigilGas",
				"ModularGameplay",
				"TargetingSystem"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"NetCore",
				"Engine",
				"Niagara",
				"AIModule",
				"DeveloperSettings"
			}
		);
	}
}