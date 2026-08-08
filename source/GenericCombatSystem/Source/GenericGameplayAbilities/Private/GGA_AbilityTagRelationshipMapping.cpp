// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GGA_AbilityTagRelationshipMapping.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GGA_AbilityTagRelationshipMapping)

void UGGA_AbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancelV2(const FGameplayTagContainer& ActorTags, const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock,
                                                                          FGameplayTagContainer* OutTagsToCancel) const
{
	TArray<FGameplayTag> AbilitiesWithLayeredRule;
	for (int32 i = 0; i < Layered.Num(); i++)
	{
		if (!ActorTags.IsEmpty() && Layered[i].ActorTagQuery.Matches(ActorTags))
		{
			const TArray<FGGA_AbilityTagRelationship>& LayeredAbilityTagRelationships = Layered[i].AbilityTagRelationships;

			// Simple iteration for now
			for (int32 j = 0; j < LayeredAbilityTagRelationships.Num(); j++)
			{
				const FGGA_AbilityTagRelationship& Tags = LayeredAbilityTagRelationships[j];
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
		const FGGA_AbilityTagRelationship& Tags = AbilityTagRelationships[i];
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

void UGGA_AbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const
{
	// Simple iteration for now
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FGGA_AbilityTagRelationship& Tags = AbilityTagRelationships[i];
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

void UGGA_AbilityTagRelationshipMapping::GetRequiredAndBlockedActivationTagsV2(const FGameplayTagContainer& ActorTags, const FGameplayTagContainer& AbilityTags,
                                                                               FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const
{
	TArray<FGameplayTag> AbilitiesWithLayeredRule;

	for (int32 i = 0; i < Layered.Num(); i++)
	{
		if (!ActorTags.IsEmpty() && Layered[i].ActorTagQuery.Matches(ActorTags))
		{
			const TArray<FGGA_AbilityTagRelationship>& LayeredAbilityTagRelationships = Layered[i].AbilityTagRelationships;

			// Simple iteration for now
			for (int32 j = 0; j < LayeredAbilityTagRelationships.Num(); j++)
			{
				const FGGA_AbilityTagRelationship& Tags = LayeredAbilityTagRelationships[j];
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
		const FGGA_AbilityTagRelationship& Tags = AbilityTagRelationships[i];
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

void UGGA_AbilityTagRelationshipMapping::GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired,
                                                                             FGameplayTagContainer* OutActivationBlocked) const
{
	// Simple iteration for now
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FGGA_AbilityTagRelationship& Tags = AbilityTagRelationships[i];
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

bool UGGA_AbilityTagRelationshipMapping::IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const
{
	// Simple iteration for now
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FGGA_AbilityTagRelationship& Tags = AbilityTagRelationships[i];

		if (Tags.AbilityTag == ActionTag && Tags.AbilityTagsToCancel.HasAny(AbilityTags))
		{
			return true;
		}
	}

	return false;
}

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"

void UGGA_AbilityTagRelationshipMapping::PreSave(FObjectPreSaveContext SaveContext)
{
	for (FGGA_AbilityTagRelationship& Rel : AbilityTagRelationships)
	{
		Rel.EditorFriendlyName = Rel.DevDescription.IsEmpty() ? Rel.AbilityTag.ToString() : Rel.DevDescription;
	}
	for (FGGA_AbilityTagRelationshipsWithQuery& RelationShips : Layered)
	{
		RelationShips.EditorFriendlyName = RelationShips.ActorTagQuery.IsEmpty() ? TEXT("Empty Query") : RelationShips.ActorTagQuery.GetDescription();
		for (FGGA_AbilityTagRelationship& AbilityTagRelationship : RelationShips.AbilityTagRelationships)
		{
			AbilityTagRelationship.EditorFriendlyName = AbilityTagRelationship.DevDescription.IsEmpty() ? AbilityTagRelationship.AbilityTag.ToString() : AbilityTagRelationship.DevDescription;
		}
	}
	Super::PreSave(SaveContext);
}
#endif
