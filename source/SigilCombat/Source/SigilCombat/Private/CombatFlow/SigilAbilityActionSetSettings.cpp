// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "CombatFlow/SigilAbilityActionSetSettings.h"

bool USigilAbilityActionSetSettings::SelectBestAbilityActions(const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, const FGameplayTagContainer& AbilityTags,
                                                             TArray<FSigilAbilityAction>& Actions) const
{
	FSigilAbilityActionSet ActionSet;
	bool bFound = false;

	for (int32 i = 0; i < ActionSets.Num(); i++)
	{
		if (ActionSets[i].AbilityTag.IsValid() && ActionSets[i].AbilityTag.MatchesAny(AbilityTags))
		{
			ActionSet = ActionSets[i];
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		return false;
	}

	// try finds in layers.

	for (int32 i = 0; i < ActionSet.Layered.Num(); i++)
	{
		bool bMatchingSource = ActionSet.Layered[i].SourceTagQuery.IsEmpty() || ActionSet.Layered[i].SourceTagQuery.Matches(SourceTags);
		bool bMatchingTarget = ActionSet.Layered[i].TargetTagQuery.IsEmpty() || ActionSet.Layered[i].TargetTagQuery.Matches(TargetTags);
		if (bMatchingSource && bMatchingTarget)
		{
			Actions = ActionSet.Layered[i].Actions;
			return true;
		}
	}

	// falback to default.
	Actions = ActionSet.Actions;

	return true;
}

#if WITH_EDITORONLY_DATA
#include "UObject/ObjectSaveContext.h"

void USigilAbilityActionSetSettings::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	for (FSigilAbilityActionSet& ActionSet : ActionSets)
	{
		for (FSigilAbilityAction& Action : ActionSet.Actions)
		{
			Action.EditorFriendlyName = Action.Animation != nullptr ? Action.Animation.GetName() : TEXT("Empty Action");
		}
		for (FSigilAbilityActionsWithQuery& Layered : ActionSet.Layered)
		{
			Layered.EditorFriendlyName = FString::Format(TEXT("Source:({0}) Target({1})"), {
				                                             Layered.SourceTagQuery.IsEmpty() ? TEXT("Empty") : Layered.SourceTagQuery.GetDescription(),
				                                             Layered.TargetTagQuery.IsEmpty() ? TEXT("Empty") : Layered.TargetTagQuery.GetDescription()
			                                             });
			for (FSigilAbilityAction& Action : Layered.Actions)
			{
				Action.EditorFriendlyName = Action.Animation != nullptr ? Action.Animation.GetName() : TEXT("Empty Action");
			}
		}
	}
}
#endif
