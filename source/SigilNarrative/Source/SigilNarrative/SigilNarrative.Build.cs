// Copyright (c) 2026 Likeon. All Rights Reserved.

using UnrealBuildTool;

public class SigilNarrative : ModuleRules
{
	public SigilNarrative(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CoreUObject",
				"Engine"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"Json",
				"JsonUtilities"
			}
		);
	}
}
