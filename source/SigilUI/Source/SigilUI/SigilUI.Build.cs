// Copyright (c) 2026 Likeon. All Rights Reserved.


using UnrealBuildTool;

public class SigilUI : ModuleRules
{
    public SigilUI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CommonUI"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "ApplicationCore",
                "EnhancedInput",
                "PropertyPath",
                "GameplayTags",
                "UMG",
                "InputCore",
                "CommonInput",
                "DeveloperSettings",
                "ModularGameplay"
            }
        );
    }
}