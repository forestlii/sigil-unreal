// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilAbilityTagRelationshipMapping.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilAbilityTagRelationshipMapping)

void USigilAbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancelV2(const FGameplayTagContainer& ActorTags, const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock,
                                                                          FGameplayTagContainer* OutTagsToCancel) const
{
	TArray<FGameplayTag> AbilitiesWithLayeredRule;
	for (int32 i = 0; i < Layered.Num(); i++)
	{
		if (!ActorTags.IsEmpty() && Layered[i].ActorTagQuery.Matches(ActorTags))
		{
			const TArray<FSigilAbilityTagRelationship>& LayeredAbilityTagRelationships = Layered[i].AbilityTagRelationships;

			// Simple iteration for now
			for (int32 j = 0; j < LayeredAbilityTagRelationships.Num(); j++)
			{
				const FSigilAbilityTagRelationship& Tags = LayeredAbilityTagRelationships[j];
				if (AbilityTags.HasTag(Tags.AbilityTag))
				{
					if (OutTagsToBlock)
					{
						OutTagsToBlock->AppendTags(Tags.AbilityTagsToBlock);
					}
					if (OutTagsToCancel)
					{
						OutTagsToCancel->AppendTags(Tags.AbilityTagsToCancel);
					}
					AbilitiesWithLayeredRule.Add(Tags.AbilityTag);
				}
			}
		}
	}

	// Simple iteration for now
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FSigilAbilityTagRelationship& Tags = AbilityTagRelationships[i];
		if (AbilityTags.HasTag(Tags.AbilityTag) && !AbilitiesWithLayeredRule.Contains(Tags.AbilityTag))
		{
			if (OutTagsToBlock)
			{
				OutTagsToBlock->AppendTags(Tags.AbilityTagsToBlock);
			}
			if (OutTagsToCancel)
			{
				OutTagsToCancel->AppendTags(Tags.AbilityTagsToCancel);
			}
		}
	}
}

void USigilAbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const
{
	// Simple iteration for now
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FSigilAbilityTagRelationship& Tags = AbilityTagRelationships[i];
		if (AbilityTags.HasTag(Tags.AbilityTag))
		{
			if (OutTagsToBlock)
			{
				OutTagsToBlock->AppendTags(Tags.AbilityTagsToBlock);
			}
			if (OutTagsToCancel)
			{
				OutTagsToCancel->AppendTags(Tags.AbilityTagsToCancel);
			}
		}
	}
}

void USigilAbilityTagRelationshipMapping::GetRequiredAndBlockedActivationTagsV2(const FGameplayTagContainer& ActorTags, const FGameplayTagContainer& AbilityTags,
                                                                               FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const
{
	TArray<FGameplayTag> AbilitiesWithLayeredRule;

	for (int32 i = 0; i < Layered.Num(); i++)
	{
		if (!ActorTags.IsEmpty() && Layered[i].ActorTagQuery.Matches(ActorTags))
		{
			const TArray<FSigilAbilityTagRelationship>& LayeredAbilityTagRelationships = Layered[i].AbilityTagRelationships;

			// Simple iteration for now
			for (int32 j = 0; j < LayeredAbilityTagRelationships.Num(); j++)
			{
				const FSigilAbilityTagRelationship& Tags = LayeredAbilityTagRelationships[j];
				if (AbilityTags.HasTag(Tags.AbilityTag))
				{
					if (OutActivationRequired)
					{
						OutActivationRequired->AppendTags(Tags.ActivationRequiredTags);
					}
					if (OutActivationBlocked)
					{
						OutActivationBlocked->AppendTags(Tags.ActivationBlockedTags);
					}
					AbilitiesWithLayeredRule.Add(Tags.AbilityTag);
				}
			}
		}
	}

	// Simple iteration for now
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FSigilAbilityTagRelationship& Tags = AbilityTagRelationships[i];
		if (AbilityTags.HasTag(Tags.AbilityTag) && !AbilitiesWithLayeredRule.Contains(Tags.AbilityTag))
		{
			if (OutActivationRequired)
			{
				OutActivationRequired->AppendTags(Tags.ActivationRequiredTags);
			}
			if (OutActivationBlocked)
			{
				OutActivationBlocked->AppendTags(Tags.ActivationBlockedTags);
			}
		}
	}
}

void USigilAbilityTagRelationshipMapping::GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired,
                                                                             FGameplayTagContainer* OutActivationBlocked) const
{
	// Simple iteration for now
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FSigilAbilityTagRelationship& Tags = AbilityTagRelationships[i];
		if (AbilityTags.HasTag(Tags.AbilityTag))
		{
			if (OutActivationRequired)
			{
				OutActivationRequired->AppendTags(Tags.ActivationRequiredTags);
			}
			if (OutActivationBlocked)
			{
				OutActivationBlocked->AppendTags(Tags.ActivationBlockedTags);
			}
		}
	}
}

bool USigilAbilityTagRelationshipMapping::IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const
{
	// Simple iteration for now
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FSigilAbilityTagRelationship& Tags = AbilityTagRelationships[i];

		if (Tags.AbilityTag == ActionTag && Tags.AbilityTagsToCancel.HasAny(AbilityTags))
		{
			return true;
		}
	}

	return false;
}

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"

void USigilAbilityTagRelationshipMapping::PreSave(FObjectPreSaveContext SaveContext)
{
	for (FSigilAbilityTagRelationship& Rel : AbilityTagRelationships)
	{
		Rel.EditorFriendlyName = Rel.DevDescription.IsEmpty() ? Rel.AbilityTag.ToString() : Rel.DevDescription;
	}
	for (FSigilAbilityTagRelationshipsWithQuery& RelationShips : Layered)
	{
		RelationShips.EditorFriendlyName = RelationShips.ActorTagQuery.IsEmpty() ? TEXT("Empty Query") : RelationShips.ActorTagQuery.GetDescription();
		for (FSigilAbilityTagRelationship& AbilityTagRelationship : RelationShips.AbilityTagRelationships)
		{
			AbilityTagRelationship.EditorFriendlyName = AbilityTagRelationship.DevDescription.IsEmpty() ? AbilityTagRelationship.AbilityTag.ToString() : AbilityTagRelationship.DevDescription;
		}
	}
	Super::PreSave(SaveContext);
}
#endif
