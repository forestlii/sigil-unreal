// Copyright 2025 RedMoonGames All Rights Reserved.


using UnrealBuildTool;

public class GenericGameplayAttributes : ModuleRules
{
    public GenericGameplayAttributes(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "GameplayTags","GameplayAbilities","GameplayTasks"
            }
        );
    }
}