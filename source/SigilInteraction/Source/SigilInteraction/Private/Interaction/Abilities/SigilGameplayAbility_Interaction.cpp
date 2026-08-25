// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Interaction/Abilities/SigilGameplayAbility_Interaction.h"
#include "Engine/World.h"
#include "SigilInteractionLogChannels.h"
#include "SmartObjectBlueprintFunctionLibrary.h"
#include "Interaction/SigilInteractionDefinition.h"
#include "Interaction/SigilInteractionSystemComponent.h"
#include "Misc/DataValidation.h"

USigilGameplayAbility_Interaction::USigilGameplayAbility_Interaction()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

void USigilGameplayAbility_Interaction::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                       const FGameplayEventData* TriggerEventData)
{
	InteractionSystem = USigilInteractionSystemComponent::GetInteractionSystemComponent(ActorInfo->AvatarActor.Get());
	if (InteractionSystem == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	InteractionSystem->OnInteractableActorChangedEvent.AddDynamic(this, &ThisClass::OnInteractActorChanged);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void USigilGameplayAbility_Interaction::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                  bool bReplicateEndAbility, bool bWasCancelled)
{
	if (USigilInteractionSystemComponent* UserComponent = USigilInteractionSystemComponent::GetInteractionSystemComponent(ActorInfo->AvatarActor.Get()))
	{
		UserComponent->OnInteractableActorChangedEvent.RemoveDynamic(this, &ThisClass::OnInteractActorChanged);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool USigilGameplayAbility_Interaction::TryClaimInteraction(int32 Index, FSmartObjectClaimHandle& ClaimedHandle)
{
	USmartObjectSubsystem* Subsystem = USmartObjectSubsystem::GetCurrent(GetWorld());

	check(Subsystem!=nullptr)
	const TArray<FSigilInteractionOption>& InteractionInstances = InteractionSystem->GetInteractionOptions();
	if (!InteractionInstances.IsValidIndex(Index))
	{
		SIGIL_INTERACTION_CLOG(Error, "Interaction at index(%d) not exist!!", Index)
		return false;
	}

	const FSigilInteractionOption CurrentOption = InteractionInstances[Index];
	if (CurrentOption.Definition == nullptr)
	{
		SIGIL_INTERACTION_CLOG(Error, "Interaction at index(%d) has invalid definition!", Index)
		return false;
	}

	if (CurrentOption.SlotState != ESmartObjectSlotState::Free)
	{
		SIGIL_INTERACTION_CLOG(Error, "Interaction(%s) was Claimed/Occupied!", *CurrentOption.Definition->Text.ToString())
		return false;
	}

	FSmartObjectClaimHandle NewlyClaimedHandle = USmartObjectBlueprintFunctionLibrary::MarkSmartObjectSlotAsClaimed(GetWorld(), CurrentOption.RequestResult.SlotHandle, GetAvatarActorFromActorInfo());

	// A valid claimed handle can point to an object that is no longer part of the simulation
	if (!Subsystem->IsClaimedSmartObjectValid(NewlyClaimedHandle))
	{
		SIGIL_INTERACTION_CLOG(Error, "Interaction(%s) refers to an object that is no longer available.!", *CurrentOption.Definition->Text.ToString())
		return false;
	}

	ClaimedHandle = NewlyClaimedHandle;
	return true;
}


void USigilGameplayAbility_Interaction::OnInteractActorChanged_Implementation(AActor* OldActor, AActor* NewActor)
{
}

#if WITH_EDITORONLY_DATA
EDataValidationResult USigilGameplayAbility_Interaction::IsDataValid(class FDataValidationContext& Context) const
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
