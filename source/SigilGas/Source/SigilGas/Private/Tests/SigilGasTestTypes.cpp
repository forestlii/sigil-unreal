// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilGasTestTypes.h"
#include "SigilGasTags.h"

namespace SigilGasTestTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SharedCooldown, "Sigil.Test.Cooldown.Shared", "Automation-only cooldown tag for the shared cooldown ability test.");
}

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
USigilGasTestSharedCooldownEffect::USigilGasTestSharedCooldownEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = SigilSetByCallerTags::CooldownDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
}

USigilGasTestFixedCooldownEffect::USigilGasTestFixedCooldownEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(2.f));
}

USigilGasTestSharedCooldownAbility::USigilGasTestSharedCooldownAbility()
{
	CooldownGameplayEffectClass = USigilGasTestSharedCooldownEffect::StaticClass();
	CooldownTags.AddTag(SigilGasTestTags::SharedCooldown);
	CooldownDuration = FScalableFloat(3.f);
}

USigilGasTestFixedCooldownAbility::USigilGasTestFixedCooldownAbility()
{
	CooldownGameplayEffectClass = USigilGasTestFixedCooldownEffect::StaticClass();
}
USigilGasTestPassiveAbility::USigilGasTestPassiveAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	bEndImmediately = false;
	SetAssetTags(FGameplayTagContainer(SigilAbilityTraitTags::ActivationOnSpawn));
}
