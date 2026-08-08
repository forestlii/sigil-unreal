using UnrealBuildTool;

public class GenericGameplayAbilitiesEditor : ModuleRules
{
    public GenericGameplayAbilitiesEditor(ReadOnlyTargetRules Target) : base(Target)
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
                "GameplayTagsEditor", "GenericGameplayAbilities"
            }
        );
    }
}