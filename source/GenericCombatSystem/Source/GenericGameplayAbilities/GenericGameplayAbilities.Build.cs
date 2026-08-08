// Copyright 2025 RedMoonGames All Rights Reserved.


using System.IO;
using UnrealBuildTool;

public class GenericGameplayAbilities : ModuleRules
{
	public GenericGameplayAbilities(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new[]
			{
				Path.Combine(ModuleDirectory, "Public"),
				Path.Combine(ModuleDirectory, "Public/Globals"),
				Path.Combine(ModuleDirectory, "Public/Abilities")
				// ... add public include paths required here ...
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
				// ... add other private include paths required here ...
			}
		);


		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core", "GameplayAbilities", "GameplayTags", "ModularGameplay",
				"GameplayTasks", "DeveloperSettings", "EnhancedInput"
				// ... add other public dependencies that you statically link with here ...
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject", "NetCore",
				"Engine", "EnhancedInput", "TargetingSystem"

				// ... add private dependencies that you statically link with here ...	
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}