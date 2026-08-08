// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilInputChecker.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "SigilInputSystemComponent.h"

bool USigilInputChecker::CheckInput(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent) const
{
	return DoCheckInput(IC, ActionData, InputTag, TriggerEvent);
}

bool USigilInputChecker::DoCheckInput_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag,
                                                    const ETriggerEvent& TriggerEvent) const
{
	return true;
}

FGameplayTagContainer USigilInputChecker_TagRelationship::GetActorTags_Implementation(USigilInputSystemComponent* IC) const
{
	FGameplayTagContainer Tags;
	if (const IGameplayTagAssetInterface* TagAssetInterface = Cast<IGameplayTagAssetInterface>(IC->GetOwner()))
	{
		TagAssetInterface->GetOwnedGameplayTags(Tags);
	}
	return Tags;
}

bool USigilInputChecker_TagRelationship::DoCheckInput_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag,
                                                                    const ETriggerEvent& TriggerEvent) const
{
	const FGameplayTagContainer ActorOwnedTags = GetActorTags(IC);

	for (const FSigilInputTagRelationship& Relationship : InputTagRelationships)
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

void USigilInputChecker_TagRelationship::PreSave(FObjectPreSaveContext SaveContext)
{
	for (FSigilInputTagRelationship& InputTagRelationship : InputTagRelationships)
	{
		InputTagRelationship.EditorFriendlyName = InputTagRelationship.ActorTagQuery.GetDescription();
	}
	Super::PreSave(SaveContext);
}
#endif
