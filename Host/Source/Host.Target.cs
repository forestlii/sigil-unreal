// Copyright (c) 2026 Likeon. All Rights Reserved.

using UnrealBuildTool;

public class HostTarget : TargetRules
{
	public HostTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Host");
	}
}
