// Copyright 2025 RedMoonGames All Rights Reserved.


#include "CombatFlow/GCS_CombatFlow.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Pawn.h"
#include "Utilities/GGA_AbilitySystemFunctionLibrary.h"
#include "GCS_CombatSystemComponent.h"
#include "CombatFlow/GCS_AttackResultProcessor.h"


UGCS_CombatFlow::UGCS_CombatFlow()
{
	Owner = nullptr;
}

void UGCS_CombatFlow::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UGCS_CombatFlow::Initialize(AActor* NewOwner)
{
	Owner = NewOwner;
	// ProcessedAttacks.SetCombatFlow(this);
	CombatComponent = UGCS_CombatSystemComponent::GetCombatSystemComponent(Owner);
}

void UGCS_CombatFlow::HandlePreGameplayEffectSpecApply_Implementation(const FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent,
                                                                      FGameplayTagContainer& OutDynamicTagsAppendToSpec)
{
}

void UGCS_CombatFlow::HandleGameplayEffectExecute_Implementation(const FGGA_GameplayEffectModCallbackData& Payload)
{
}

void UGCS_CombatFlow::HandleAttackResult_Implementation(const FGCS_AttackResult& InPayload)
{
	CombatComponent->SetLastProcessedAttackResult(InPayload);

	for (TObjectPtr<UGCS_AttackResultProcessor>& Processor : AttackResultProcessors)
	{
		if (IsValid(Processor))
		{
			Processor->ProcessIncomingAttackResult(InPayload);
		}
	}
}
