// Copyright (c) 2026 Likeon. All Rights Reserved.

using UnrealBuildTool;

public class HostEditorTarget : TargetRules
{
	public HostEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Host");
	}
}
