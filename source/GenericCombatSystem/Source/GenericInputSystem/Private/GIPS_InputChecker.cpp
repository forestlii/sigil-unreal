// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIPS_InputChecker.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "GIPS_InputSystemComponent.h"

bool UGIPS_InputChecker::CheckInput(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent) const
{
	return DoCheckInput(IC, ActionData, InputTag, TriggerEvent);
}

bool UGIPS_InputChecker::DoCheckInput_Implementation(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag,
                                                    const ETriggerEvent& TriggerEvent) const
{
	return true;
}

FGameplayTagContainer UGIPS_InputChecker_TagRelationship::GetActorTags_Implementation(UGIPS_InputSystemComponent* IC) const
{
	FGameplayTagContainer Tags;
	if (const IGameplayTagAssetInterface* TagAssetInterface = Cast<IGameplayTagAssetInterface>(IC->GetOwner()))
	{
		TagAssetInterface->GetOwnedGameplayTags(Tags);
	}
	return Tags;
}

bool UGIPS_InputChecker_TagRelationship::DoCheckInput_Implementation(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag,
                                                                    const ETriggerEvent& TriggerEvent) const
{
	const FGameplayTagContainer ActorOwnedTags = GetActorTags(IC);

	for (const FGIPS_InputTagRelationship& Relationship : InputTagRelationships)
	{
		// TagQuery > TagRequirements.
		if (Relationship.ActorTagQuery.IsEmpty() || !Relationship.ActorTagQuery.Matches(ActorOwnedTags))
		{
			continue;
		}

		int32 Index = Relationship.IndexOfAllowedInput(InputTag, TriggerEvent);
		if (Index != INDEX_NONE)
		{
			return true;
		}
		// if (Relationship.InputTagsAllowed.HasTag(InputTag))
		// {
		// 	return true;
		// }

		return false;
	}

	return true;
}

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"

void UGIPS_InputChecker_TagRelationship::PreSave(FObjectPreSaveContext SaveContext)
{
	for (FGIPS_InputTagRelationship& InputTagRelationship : InputTagRelationships)
	{
		InputTagRelationship.EditorFriendlyName = InputTagRelationship.ActorTagQuery.GetDescription();
	}
	Super::PreSave(SaveContext);
}
#endif
