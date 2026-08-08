// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Interaction/Abilities/GGS_GameplayAbility_Interaction.h"
#include "Engine/World.h"
#include "GGS_LogChannels.h"
#include "SmartObjectBlueprintFunctionLibrary.h"
#include "Interaction/GGS_InteractionDefinition.h"
#include "Interaction/GGS_InteractionSystemComponent.h"
#include "Misc/DataValidation.h"

UGGS_GameplayAbility_Interaction::UGGS_GameplayAbility_Interaction()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

void UGGS_GameplayAbility_Interaction::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                       const FGameplayEventData* TriggerEventData)
{
	InteractionSystem = UGGS_InteractionSystemComponent::GetInteractionSystemComponent(ActorInfo->AvatarActor.Get());
	if (InteractionSystem == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	InteractionSystem->OnInteractableActorChangedEvent.AddDynamic(this, &ThisClass::OnInteractActorChanged);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGGS_GameplayAbility_Interaction::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                  bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UGGS_InteractionSystemComponent* UserComponent = UGGS_InteractionSystemComponent::GetInteractionSystemComponent(ActorInfo->AvatarActor.Get()))
	{
		UserComponent->OnInteractableActorChangedEvent.RemoveDynamic(this, &ThisClass::OnInteractActorChanged);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UGGS_GameplayAbility_Interaction::TryClaimInteraction(int32 Index, FSmartObjectClaimHandle& ClaimedHandle)
{
	USmartObjectSubsystem* Subsystem = USmartObjectSubsystem::GetCurrent(GetWorld());

	check(Subsystem!=nullptr)
	const TArray<FGGS_InteractionOption>& InteractionInstances = InteractionSystem->GetInteractionOptions();
	if (!InteractionInstances.IsValidIndex(Index))
	{
		GGS_CLOG(Error, "Interaction at index(%d) not exist!!", Index)
		return false;
	}

	if (InteractionInstances[Index].Definition == nullptr)
	{
		GGS_CLOG(Error, "Interaction at index(%d) has invalid definition!", Index)
		return false;
	}

	if (InteractionInstances[Index].SlotState != ESmartObjectSlotState::Free)
	{
		GGS_CLOG(Error, "Interaction(%s) was Claimed/Occupied!", *InteractionInstances[Index].Definition->Text.ToString())
		return false;
	}

	const FGGS_InteractionOption& CurrentOption = InteractionInstances[Index];

	FSmartObjectClaimHandle NewlyClaimedHandle = USmartObjectBlueprintFunctionLibrary::MarkSmartObjectSlotAsClaimed(GetWorld(), CurrentOption.RequestResult.SlotHandle, GetAvatarActorFromActorInfo());

	// A valid claimed handle can point to an object that is no longer part of the simulation
	if (!Subsystem->IsClaimedSmartObjectValid(NewlyClaimedHandle))
	{
		GGS_CLOG(Error, "Interaction(%s) refers to an object that is no longer available.!", *InteractionInstances[Index].Definition->Text.ToString())
		return false;
	}

	ClaimedHandle = NewlyClaimedHandle;
	return true;
}


void UGGS_GameplayAbility_Interaction::OnInteractActorChanged_Implementation(AActor* OldActor, AActor* NewActor)
{
}

#if WITH_EDITORONLY_DATA
EDataValidationResult UGGS_GameplayAbility_Interaction::IsDataValid(class FDataValidationContext& Context) const
{
	if (ReplicationPolicy != EGameplayAbilityReplicationPolicy::ReplicateYes)
	{
		Context.AddError(FText::FromString(TEXT("Core Interaction ability must be Replicated to allow client->server communications via RPC.")));
		return EDataValidationResult::Invalid;
	}
	if (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly || NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly)
	{
		Context.AddError(FText::FromString(TEXT("Core Interaction ability must not be Local/Server only.")));
		return EDataValidationResult::Invalid;
	}
	if (!AbilityTriggers.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("Core Interaction ability doesn't allow event triggering!")));
		return EDataValidationResult::Invalid;
	}
	if (InstancingPolicy != EGameplayAbilityInstancingPolicy::InstancedPerActor)
	{
		Context.AddError(FText::FromString(TEXT("Core Interaction ability's instancing policy must be InstancedPerActor")));
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}
#endif
