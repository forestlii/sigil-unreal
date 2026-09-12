// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilGasTestTypes.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "SigilGasTags.h"

namespace SigilGasTestTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SharedCooldown, "Sigil.Test.Cooldown.Shared", "Automation-only cooldown tag for the shared cooldown ability test.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SharedCooldownB, "Sigil.Test.Cooldown.SharedB", "Automation-only cooldown tag for the second ability sharing the cooldown effect.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(FixedCooldown, "Sigil.Test.Cooldown.Fixed", "Automation-only cooldown tag granted by the fixed-duration cooldown effect.");
}

namespace
{
// FindOrAddComponent uses NewObject, which is not allowed inside a UObject constructor; build the component as a default
// subobject and register it in the (protected) GEComponents array through the CDO's own constructor instead.
#define SIGIL_GAS_TEST_GRANT_TAG(Effect, Tag)                                                                                             	{                                                                                                                                     		UTargetTagsGameplayEffectComponent* TargetTags = (Effect)->CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("SigilTestTargetTags")); 		(Effect)->GEComponents.Add(TargetTags);                                                                                           		FInheritedTagContainer TagChanges;                                                                                                		TagChanges.Added.AddTag(Tag);                                                                                                     		TargetTags->SetAndApplyTargetTagChanges(TagChanges);                                                                              	}
}

ASigilGasTestAbilityActor::ASigilGasTestAbilityActor()
{
	PrimaryActorTick.bCanEverTick = false;
	AbilitySystem = CreateDefaultSubobject<USigilGasTestAbilitySystemComponent>(TEXT("AbilitySystem"));
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
	// Shared cooldown effects grant the marker so the engine's IsDataValid accepts them; it must never gate activation.
	SIGIL_GAS_TEST_GRANT_TAG(this, SigilCooldownTags::SharedMarker);
}

USigilGasTestFixedCooldownEffect::USigilGasTestFixedCooldownEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(2.f));
	SIGIL_GAS_TEST_GRANT_TAG(this, SigilGasTestTags::FixedCooldown);
}

USigilGasTestSharedCooldownAbility::USigilGasTestSharedCooldownAbility()
{
	CooldownGameplayEffectClass = USigilGasTestSharedCooldownEffect::StaticClass();
	CooldownTags.AddTag(SigilGasTestTags::SharedCooldown);
	CooldownDuration = FScalableFloat(3.f);
}

USigilGasTestSharedCooldownAbilityB::USigilGasTestSharedCooldownAbilityB()
{
	CooldownGameplayEffectClass = USigilGasTestSharedCooldownEffect::StaticClass();
	CooldownTags.AddTag(SigilGasTestTags::SharedCooldownB);
	CooldownDuration = FScalableFloat(4.f);
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
