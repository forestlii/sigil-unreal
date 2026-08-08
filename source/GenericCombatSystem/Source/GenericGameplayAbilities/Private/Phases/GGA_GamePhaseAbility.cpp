// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Phases/GGA_GamePhaseAbility.h"
#include "AbilitySystemComponent.h"
#include "Phases/GGA_GamePhaseSubsystem.h"
#include "Engine/World.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif


#include UE_INLINE_GENERATED_CPP_BY_NAME(GGA_GamePhaseAbility)

#define LOCTEXT_NAMESPACE "UGGA_GamePhaseAbility"

UGGA_GamePhaseAbility::UGGA_GamePhaseAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;
}

void UGGA_GamePhaseAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	if (ActorInfo->IsNetAuthority())
	{
		UWorld* World = ActorInfo->AbilitySystemComponent->GetWorld();
		UGGA_GamePhaseSubsystem* PhaseSubsystem = UWorld::GetSubsystem<UGGA_GamePhaseSubsystem>(World);
		PhaseSubsystem->OnBeginPhase(this, Handle);
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGGA_GamePhaseAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                       bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo->IsNetAuthority())
	{
		UWorld* World = ActorInfo->AbilitySystemComponent->GetWorld();
		UGGA_GamePhaseSubsystem* PhaseSubsystem = UWorld::GetSubsystem<UGGA_GamePhaseSubsystem>(World);
		PhaseSubsystem->OnEndPhase(this, Handle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

#if WITH_EDITOR
#if ENGINE_MINOR_VERSION > 2
EDataValidationResult UGGA_GamePhaseAbility::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	if (!GamePhaseTag.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("GamePhaseTagNotSet", "GamePhaseTag must be set to a tag representing the current phase."));
	}

	return Result;
}
#endif
#endif

#undef LOCTEXT_NAMESPACE
