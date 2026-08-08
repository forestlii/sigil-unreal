// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Interaction/SigilInteractionSystemComponent.h"
#include "Engine/World.h"
#include "GameplayBehaviorSmartObjectBehaviorDefinition.h"
#include "SigilInteractionLogChannels.h"
#include "SmartObjectComponent.h"
#include "SmartObjectSubsystem.h"
#include "Interaction/SigilInteractableInterface.h"
#include "Interaction/SigilSmartObjectFunctionLibrary.h"
#include "Interaction/SigilInteractionStructLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

// Sets default values for this component's properties
USigilInteractionSystemComponent::USigilInteractionSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USigilInteractionSystemComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_OwnerOnly;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InteractableActor, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, NumsOfInteractableActors, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InteractingOption, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InteractionOptions, Params);
}

USigilInteractionSystemComponent* USigilInteractionSystemComponent::GetInteractionSystemComponent(const AActor* Actor)
{
	return IsValid(Actor) ? Actor->FindComponentByClass<USigilInteractionSystemComponent>() : nullptr;
}

void USigilInteractionSystemComponent::CycleInteractableActors_Implementation(bool bNext)
{
	if (bInteracting || InteractableActors.Num() <= 1)
	{
		return;
	}

	int32 Index = InteractableActor != nullptr ? InteractableActors.IndexOfByKey(InteractableActor) : 0;
	if (!InteractableActors.IsValidIndex(Index)) //一个都没
	{
		return;
	}
	if (bNext)
	{
		Index = FMath::Clamp(Index + 1, 0, InteractableActors.Num());
	}
	else
	{
		Index = FMath::Clamp(Index - 1, 0, InteractableActors.Num());
	}
	if (InteractableActors.IsValidIndex(Index) && InteractableActors[Index] != nullptr && InteractableActors[Index] !=
		InteractableActor)
	{
		SetInteractableActor(InteractableActors[Index]);
	}
}

void USigilInteractionSystemComponent::SearchInteractableActors()
{
	OnSearchInteractableActorsEvent.Broadcast();
}

void USigilInteractionSystemComponent::SetInteractableActors(TArray<AActor*> NewActors)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	InteractableActors = NewActors;
	SetInteractableActorsNum(InteractableActors.Num());
	OnInteractableActorsChanged();
}

void USigilInteractionSystemComponent::SetInteractableActorsNum(int32 NewNum)
{
	if (NewNum != NumsOfInteractableActors)
	{
		const int32 PrevNum = NumsOfInteractableActors;
		NumsOfInteractableActors = NewNum;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, NumsOfInteractableActors, this)
		OnInteractableActorsNumChanged();
	}
}

void USigilInteractionSystemComponent::SetInteractableActor(AActor* InActor)
{
	if (InActor != InteractableActor)
	{
		AActor* OldActor = InteractableActor;
		InteractableActor = InActor;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InteractableActor, this)
		OnInteractableActorChanged(OldActor);
	}
}

FSmartObjectRequestFilter USigilInteractionSystemComponent::GetSmartObjectRequestFilter_Implementation()
{
	return DefaultRequestFilter;
}

void USigilInteractionSystemComponent::StartInteraction(int32 NewIndex)
{
	if (bInteracting)
	{
		SIGIL_INTERACTION_CLOG(Warning, "Can't start interaction(%d) while already interacting(%d)", NewIndex, InteractingOption)
		return;
	}

	if (!InteractionOptions.IsValidIndex(NewIndex))
	{
		SIGIL_INTERACTION_CLOG(Warning, "Try start invalid interaction(%d)", NewIndex)
		return;
	}

	int32 Prev = InteractingOption;
	InteractingOption = NewIndex;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InteractingOption, this);
	OnInteractingOptionChanged(Prev);
}

void USigilInteractionSystemComponent::EndInteraction()
{
	if (!bInteracting)
	{
		//SIGIL_INTERACTION_CLOG(Warning, TEXT("no need to end interaction when there's no any active interaction."))
		return;
	}

	int32 Prev = InteractingOption;
	InteractingOption = INDEX_NONE;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InteractingOption, this);
	OnInteractingOptionChanged(Prev);
}

void USigilInteractionSystemComponent::InstantInteraction(int32 NewIndex)
{
	if (bInteracting)
	{
		SIGIL_INTERACTION_CLOG(Warning, "Can't trigger instant interaction(%d) while already interacting(%d)", NewIndex, InteractingOption)
		return;
	}
	if (!InteractionOptions.IsValidIndex(NewIndex))
	{
		SIGIL_INTERACTION_CLOG(Warning, "Try trigger invalid interaction(%d)", NewIndex)
		return;
	}

	int32 Prev = InteractingOption;
	InteractingOption = NewIndex;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InteractingOption, this);
	OnInteractingOptionChanged(Prev);

	int32 Prev2 = InteractingOption;
	InteractingOption = INDEX_NONE;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InteractingOption, this);
	OnInteractingOptionChanged(Prev2);
}

bool USigilInteractionSystemComponent::IsInteracting() const
{
	return bInteracting;
}

int32 USigilInteractionSystemComponent::GetInteractingOption() const
{
	return InteractingOption;
}

void USigilInteractionSystemComponent::OnInteractableActorChanged(AActor* OldActor)
{
	if (GetOwner()->GetLocalRole() >= ROLE_Authority)
	{
		RefreshOptionsForActor();
	}

	if (IsValid(OldActor) && OldActor->GetClass()->ImplementsInterface(USigilInteractableInterface::StaticClass()))
	{
		ISigilInteractableInterface::Execute_OnInteractionDeselected(OldActor, GetOwner());
	}

	if (IsValid(InteractableActor) && InteractableActor->GetClass()->ImplementsInterface(USigilInteractableInterface::StaticClass()))
	{
		ISigilInteractableInterface::Execute_OnInteractionSelected(InteractableActor, GetOwner());
	}

	OnInteractableActorChangedEvent.Broadcast(OldActor, InteractableActor);
}

void USigilInteractionSystemComponent::OnInteractableActorsNumChanged()
{
	OnInteractableActorNumChangedEvent.Broadcast(NumsOfInteractableActors);
}

void USigilInteractionSystemComponent::OnInteractableActorsChanged_Implementation()
{
	if (!bInteracting)
	{
		// update potential actor.
		if (!IsValid(InteractableActor) || !InteractableActors.Contains(InteractableActor))
		{
			if (InteractableActors.IsValidIndex(0) && IsValid(InteractableActors[0]))
			{
				SetInteractableActor(InteractableActors[0]);
			}
			else
			{
				SetInteractableActor(nullptr);
			}
		}

		if (bNewActorHasPriority)
		{
			if (IsValid(InteractableActor) && InteractableActors.IsValidIndex(0) && InteractableActors[0] != InteractableActor)
			{
				SetInteractableActor(InteractableActors[0]);
			}
		}
	}
}

void USigilInteractionSystemComponent::OnSmartObjectEventCallback(const FSmartObjectEventData& EventData)
{
	check(InteractableActor != nullptr);

	for (int32 i = 0; i < InteractionOptions.Num(); i++)
	{
		const FSigilInteractionOption& Option = InteractionOptions[i];
		if (EventData.SmartObjectHandle == Option.RequestResult.SmartObjectHandle && EventData.SlotHandle == Option.RequestResult.SlotHandle)
		{
			if (EventData.Reason == ESmartObjectChangeReason::OnOccupied || EventData.Reason == ESmartObjectChangeReason::OnReleased || EventData.Reason == ESmartObjectChangeReason::OnClaimed)
			{
				RefreshOptionsForActor();
			}
		}
	}
}

void USigilInteractionSystemComponent::OnInteractionOptionsChanged()
{
	for (FSigilInteractionOption& InteractionOption : InteractionOptions)
	{
		SIGIL_INTERACTION_CLOG(Verbose, "Available Options:%s", *InteractionOption.ToString())
	}
	OnInteractionOptionsChangedEvent.Broadcast();
}

void USigilInteractionSystemComponent::OnInteractingOptionChanged_Implementation(int32 PrevOptionIndex)
{
	bool bPrevInteracting = bInteracting;
	bInteracting = InteractingOption != INDEX_NONE;

	if (IsValid(InteractableActor) && InteractableActor->GetClass()->ImplementsInterface(USigilInteractableInterface::StaticClass()))
	{
		if (!bPrevInteracting && bInteracting)
		{
			ISigilInteractableInterface::Execute_OnInteractionStarted(InteractableActor, GetOwner(), InteractingOption);
		}
		if (bPrevInteracting && !bInteracting)
		{
			ISigilInteractableInterface::Execute_OnInteractionEnded(InteractableActor, GetOwner(), PrevOptionIndex);
		}
	}

	OnInteractingStateChangedEvent.Broadcast(bInteracting);
}

void USigilInteractionSystemComponent::RefreshOptionsForActor()
{
	USmartObjectSubsystem* Subsystem = USmartObjectSubsystem::GetCurrent(GetWorld());

	if (!Subsystem)
	{
		return;
	}

	// getting new options for current interact actor.
	TArray<FSigilInteractionOption> NewOptions;
	{
		TArray<FSmartObjectRequestResult> Results;
		if (IsValid(InteractableActor) && USigilSmartObjectFunctionLibrary::FindSmartObjectsWithInteractionEntranceInActor(GetSmartObjectRequestFilter(), InteractableActor, Results, GetOwner()))
		{
			for (int32 i = 0; i < Results.Num(); i++)
			{
				FSigilInteractionOption Option;
				USigilInteractionDefinition* FoundDefinition;
				if (USigilSmartObjectFunctionLibrary::FindInteractionDefinitionFromSmartObjectSlot(this, Results[i].SlotHandle, FoundDefinition))
				{
					Option.Definition = FoundDefinition;
					Option.SlotState = Subsystem->GetSlotState(Results[i].SlotHandle);
					Option.RequestResult = Results[i];
					Option.SlotIndex = i;
					Option.BehaviorDefinition = Subsystem->GetBehaviorDefinitionByRequestResult(Results[i], USmartObjectBehaviorDefinition::StaticClass());
					NewOptions.Add(Option);
				}
			}
		}
	}

	// check any options changed.
	bool bOptionsChanged = false;
	{
		if (NewOptions.Num() == InteractionOptions.Num())
		{
			NewOptions.Sort();

			for (int OptionIndex = 0; OptionIndex < NewOptions.Num(); OptionIndex++)
			{
				const FSigilInteractionOption& NewOption = NewOptions[OptionIndex];
				const FSigilInteractionOption& CurrentOption = InteractionOptions[OptionIndex];

				if (NewOption != CurrentOption)
				{
					bOptionsChanged = true;
					break;
				}
			}
		}
		else
		{
			bOptionsChanged = true;
		}
	}

	if (bOptionsChanged)
	{
		// unregister event callbacks for existing options.
		for (int32 i = 0; i < InteractionOptions.Num(); i++)
		{
			auto& Handle = InteractionOptions[i].RequestResult.SlotHandle;
			if (SlotCallbacks.Contains(Handle))
			{
				if (FOnSmartObjectEvent* OnEventDelegate = Subsystem->GetSlotEventDelegate(Handle))
				{
					OnEventDelegate->Remove(SlotCallbacks[Handle]);
					SlotCallbacks.Remove(Handle);
				}
			}
		}

		InteractionOptions = NewOptions;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InteractionOptions, this)

		SIGIL_INTERACTION_CLOG(Verbose, "Interaction options changed, nums of options:%d", InteractionOptions.Num())

		// register slot event callbacks.
		for (int32 i = 0; i < InteractionOptions.Num(); i++)
		{
			auto& Handle = InteractionOptions[i].RequestResult.SlotHandle;
			if (FOnSmartObjectEvent* OnEventDelegate = Subsystem->GetSlotEventDelegate(Handle))
			{
				FDelegateHandle DelegateHandle = OnEventDelegate->AddUObject(this, &ThisClass::OnSmartObjectEventCallback);
				SlotCallbacks.Emplace(Handle, DelegateHandle);
			}
		}

		OnInteractionOptionsChanged();
	}
}
