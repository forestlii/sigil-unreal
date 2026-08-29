// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Abilities/SigilAbilityCost.h"

bool USigilAbilityCost::CheckCost(const UGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 FGameplayTagContainer* OptionalRelevantTags) const
{
	FGameplayTagContainer EmptyRelevantTags;
	const FGameplayTagContainer& RelevantTags = OptionalRelevantTags ? *OptionalRelevantTags : EmptyRelevantTags;
	return BlueprintCheckCost(Ability, Handle, *ActorInfo, RelevantTags);
}

void USigilAbilityCost::ApplyCost(const UGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo)
{
	return BlueprintApplyCost(Ability, Handle, *ActorInfo, ActivationInfo);
}
