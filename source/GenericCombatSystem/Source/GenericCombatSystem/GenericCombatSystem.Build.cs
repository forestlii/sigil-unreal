// Copyright 2025 RedMoonGames All Rights Reserved.

using UnrealBuildTool;

public class GenericCombatSystem : ModuleRules
{
	public GenericCombatSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayTags",
				"GameplayTasks",
				"GameplayAbilities",
				"GenericGameplayAbilities",
				"GenericGameplayAttributes",
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