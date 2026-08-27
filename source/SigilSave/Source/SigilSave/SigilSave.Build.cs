// Copyright (c) 2026 Likeon. All Rights Reserved.

using UnrealBuildTool;

public class SigilSave : ModuleRules
{
	public SigilSave(ReadOnlyTargetRules Target) : base(Target)
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

		PrivateDependencyModuleNames.Add("Json");
	}
}
