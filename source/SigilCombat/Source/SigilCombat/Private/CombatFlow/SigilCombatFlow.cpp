// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "CombatFlow/SigilCombatFlow.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Pawn.h"
#include "Utilities/SigilAbilitySystemFunctionLibrary.h"
#include "SigilCombatSystemComponent.h"
#include "CombatFlow/SigilAttackResultProcessor.h"


USigilCombatFlow::USigilCombatFlow()
{
	Owner = nullptr;
}

void USigilCombatFlow::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void USigilCombatFlow::Initialize(AActor* NewOwner)
{
	Owner = NewOwner;
	CombatComponent = USigilCombatSystemComponent::GetCombatSystemComponent(Owner);
}

void USigilCombatFlow::HandlePreGameplayEffectSpecApply_Implementation(const FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent,
                                                                      FGameplayTagContainer& OutDynamicTagsAppendToSpec)
{
}

void USigilCombatFlow::HandleGameplayEffectExecute_Implementation(const FSigilGameplayEffectModCallbackData& Payload)
{
}

void USigilCombatFlow::HandleAttackResult_Implementation(const FSigilAttackResult& InPayload)
{
	CombatComponent->SetLastProcessedAttackResult(InPayload);

	for (TObjectPtr<USigilAttackResultProcessor>& Processor : AttackResultProcessors)
	{
		if (IsValid(Processor))
		{
			Processor->ProcessIncomingAttackResult(InPayload);
		}
	}
}
