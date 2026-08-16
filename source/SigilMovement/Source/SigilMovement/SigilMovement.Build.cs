// Copyright (c) 2026 Likeon. All Rights Reserved.

using UnrealBuildTool;

public class SigilMovement : ModuleRules
{
	public SigilMovement(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		bEnableNonInlinedGenCppWarnings = true;
		 
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"GameplayTags",
				"AnimationWarpingRuntime", 
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
				"AnimationLocomotionLibraryRuntime"
			}
		);

		if (Target.Type == TargetRules.TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(new[] { "MessageLog" });
		}
	}
}