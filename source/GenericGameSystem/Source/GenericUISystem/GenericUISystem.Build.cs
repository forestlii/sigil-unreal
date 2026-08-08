// Copyright 2025 RedMoonGames All Rights Reserved.


using UnrealBuildTool;

public class GenericUISystem : ModuleRules
{
    public GenericUISystem(ReadOnlyTargetRules Target) : base(Target)
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