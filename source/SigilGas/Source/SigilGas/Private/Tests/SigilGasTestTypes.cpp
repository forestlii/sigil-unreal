// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilGasTestTypes.h"

ASigilGasTestAbilityActor::ASigilGasTestAbilityActor()
{
	PrimaryActorTick.bCanEverTick = false;
	AbilitySystem = CreateDefaultSubobject<USigilAbilitySystemComponent>(TEXT("AbilitySystem"));
}

USigilGasTestAbility::USigilGasTestAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void USigilGasTestAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/ true, /*bWasCancelled*/ true);
		return;
	}

	++ActivationCount;

	if (bEndImmediately)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/ true, /*bWasCancelled*/ false);
	}
}
