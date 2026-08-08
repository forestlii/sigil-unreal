// Copyright (c) 2026 Likeon. All Rights Reserved.

using UnrealBuildTool;

public class SigilGasEditor : ModuleRules
{
    public SigilGasEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "UnrealEd",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "GameplayAbilities",
                "GameplayAbilitiesEditor",
                "PropertyEditor",
                "GameplayTasks",
                "GameplayTasksEditor",
                "GameplayTags",
                "ToolMenus",
                "AssetDefinition",
                "GameplayTagsEditor", "SigilGas"
            }
        );
    }
}