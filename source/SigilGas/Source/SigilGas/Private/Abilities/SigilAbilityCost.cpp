// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Abilities/SigilAbilityCost.h"

bool USigilAbilityCost::CheckCost(const UGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 FGameplayTagContainer* OptionalRelevantTags) const
{
	return BlueprintCheckCost(Ability, Handle, *ActorInfo, *OptionalRelevantTags);
}

void USigilAbilityCost::ApplyCost(const UGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo)
{
	return BlueprintApplyCost(Ability, Handle, *ActorInfo, ActivationInfo);
}
