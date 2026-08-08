// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Actions/GIPS_AsyncAction_ListenInputEvent.h"
#include "Engine/Engine.h"
#include "GIPS_InputSystemComponent.h"

UGIPS_AsyncAction_ListenInputEvent* UGIPS_AsyncAction_ListenInputEvent::ListenInputEvent(UObject* WorldContextObject, UGIPS_InputSystemComponent* InputSystemComponent,
                                                                                         FGameplayTagContainer InputTagsToListen, TArray<ETriggerEvent> EventsToListen, bool bListenForBufferedInput,
                                                                                         bool bExactMatch)
{
	if (!IsValid(InputSystemComponent))
	{
		FFrame::KismetExecutionMessage(TEXT("ListenInputEvent was passed a null InputSystemComponent"), ELogVerbosity::Error);
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	UGIPS_AsyncAction_ListenInputEvent* Action = NewObject<UGIPS_AsyncAction_ListenInputEvent>();
	Action->Input = InputSystemComponent;
	Action->InputTags = InputTagsToListen;
	Action->bForBufferedInput = bListenForBufferedInput;
	Action->TriggerEvents = EventsToListen;
	Action->RegisterWithGameInstance(World);

	return Action;
}

void UGIPS_AsyncAction_ListenInputEvent::Activate()
{
	if (bForBufferedInput)
	{
		Input->OnFireBufferedInput.AddDynamic(this, &ThisClass::HandleInput);
	}
	else
	{
		Input->OnReceivedInput.AddDynamic(this, &ThisClass::HandleInput);
	}
}

void UGIPS_AsyncAction_ListenInputEvent::HandleInput(const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
{
	if (bExact ? InputTags.HasTagExact(InputTag) : InputTags.HasTag(InputTag))
	{
		if (TriggerEvents.Contains(TriggerEvent))
		{
			OnReceivedInput.Broadcast(ActionData, InputTag, TriggerEvent);
		}
	}
}

void UGIPS_AsyncAction_ListenInputEvent::Cancel()
{
	if (Input.IsValid())
	{
		if (bForBufferedInput)
		{
			if (Input->OnFireBufferedInput.IsAlreadyBound(this, &ThisClass::HandleInput))
			{
				Input->OnFireBufferedInput.RemoveDynamic(this, &ThisClass::HandleInput);
			}
		}
		else
		{
			if (Input->OnFireBufferedInput.IsAlreadyBound(this, &ThisClass::HandleInput))
			{
				Input->OnFireBufferedInput.RemoveDynamic(this, &ThisClass::HandleInput);
			}
		}
		Super::Cancel();
	}
}
