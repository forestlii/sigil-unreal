// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Actions/SigilAsyncAction_ListenInputEvent.h"
#include "Engine/Engine.h"
#include "SigilInputSystemComponent.h"

USigilAsyncAction_ListenInputEvent* USigilAsyncAction_ListenInputEvent::ListenInputEvent(UObject* WorldContextObject, USigilInputSystemComponent* InputSystemComponent,
                                                                                         FGameplayTagContainer InputTagsToListen, TArray<ETriggerEvent> EventsToListen, bool bListenForBufferedInput,
                                                                                         bool bExactMatch)
{
	if (!IsValid(InputSystemComponent))
	{
		FFrame::KismetExecutionMessage(TEXT("ListenInputEvent was passed a null InputSystemComponent"), ELogVerbosity::Error);
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	USigilAsyncAction_ListenInputEvent* Action = NewObject<USigilAsyncAction_ListenInputEvent>();
	Action->Input = InputSystemComponent;
	Action->InputTags = InputTagsToListen;
	Action->bForBufferedInput = bListenForBufferedInput;
	Action->TriggerEvents = EventsToListen;
	Action->RegisterWithGameInstance(World);

	return Action;
}

void USigilAsyncAction_ListenInputEvent::Activate()
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

void USigilAsyncAction_ListenInputEvent::HandleInput(const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
{
	if (bExact ? InputTags.HasTagExact(InputTag) : InputTags.HasTag(InputTag))
	{
		if (TriggerEvents.Contains(TriggerEvent))
		{
			OnReceivedInput.Broadcast(ActionData, InputTag, TriggerEvent);
		}
	}
}

void USigilAsyncAction_ListenInputEvent::Cancel()
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
