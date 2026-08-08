// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Abilities/GGA_AbilityCost.h"

bool UGGA_AbilityCost::CheckCost(const UGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 FGameplayTagContainer* OptionalRelevantTags) const
{
	return BlueprintCheckCost(Ability, Handle, *ActorInfo, *OptionalRelevantTags);
}

void UGGA_AbilityCost::ApplyCost(const UGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo)
{
	return BlueprintApplyCost(Ability, Handle, *ActorInfo, ActivationInfo);
}
