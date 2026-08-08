// Copyright (c) 2026 Likeon. All Rights Reserved.

using UnrealBuildTool;

public class Host : ModuleRules
{
	public Host(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
	}
}
