// Copyright 2025 RedMoonGames All Rights Reserved.


#include "CombatFlow/GCS_AbilityActionSetSettings.h"

bool UGCS_AbilityActionSetSettings::SelectBestAbilityActions(const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, const FGameplayTagContainer& AbilityTags,
                                                             TArray<FGCS_AbilityAction>& Actions) const
{
	FGCS_AbilityActionSet ActionSet;
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

void UGCS_AbilityActionSetSettings::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	for (FGCS_AbilityActionSet& ActionSet : ActionSets)
	{
		for (FGCS_AbilityAction& Action : ActionSet.Actions)
		{
			Action.EditorFriendlyName = Action.Animation != nullptr ? Action.Animation.GetName() : TEXT("Empty Action");
		}
		for (FGCS_AbilityActionsWithQuery& Layered : ActionSet.Layered)
		{
			Layered.EditorFriendlyName = FString::Format(TEXT("Source:({0}) Target({1})"), {
				                                             Layered.SourceTagQuery.IsEmpty() ? TEXT("Empty") : Layered.SourceTagQuery.GetDescription(),
				                                             Layered.TargetTagQuery.IsEmpty() ? TEXT("Empty") : Layered.TargetTagQuery.GetDescription()
			                                             });
			for (FGCS_AbilityAction& Action : Layered.Actions)
			{
				Action.EditorFriendlyName = Action.Animation != nullptr ? Action.Animation.GetName() : TEXT("Empty Action");
			}
		}
	}
}
#endif
