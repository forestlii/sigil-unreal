// Copyright (c) 2026 Likeon. All Rights Reserved.

using UnrealBuildTool;

public class SigilArsenal : ModuleRules
{
	public SigilArsenal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayTags",
				"GameplayAbilities",
				"SigilGas",
				"SigilInventory",
				"SigilCombat"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"NetCore"
			}
		);
	}
}
