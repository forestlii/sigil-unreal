// Copyright 2024 RedMoonGames All Rights Reserved.

using UnrealBuildTool;

public class GenericMovementSystem : ModuleRules
{
	public GenericMovementSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		bEnableNonInlinedGenCppWarnings = true;
		 
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"GameplayTags",
				"AnimationWarpingRuntime", 
				"StructUtils",
				"Chooser", "PoseSearch"
				// ... add other public dependencies that you statically link with here ...
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"NetCore",
				"ModularGameplay",
				"Engine",
				"AnimGraphRuntime",
				"BlendStack",
				"AnimationLocomotionLibraryRuntime", 
				"Niagara", 
				"DeveloperSettings"
				// ... add private dependencies that you statically link with here ...	
			}
		);

		if (Target.Type == TargetRules.TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(new[] { "MessageLog" });
		}
	}
}