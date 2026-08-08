// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIPS_InputProcessor.h"

#include "EnhancedInputComponent.h"
#include "GIPS_InputChecker.h"
#include "GIPS_InputFunctionLibrary.h"


UGIPS_InputProcessor::UGIPS_InputProcessor()
{
}

bool UGIPS_InputProcessor::CanHandleInput(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent) const
{
	return CheckCanHandleInput(IC, ActionData, InputTag, TriggerEvent);
}

void UGIPS_InputProcessor::HandleInput(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent) const
{
	switch (TriggerEvent)
	{
	case ETriggerEvent::Triggered:
		return HandleInputTriggered(IC, ActionData, InputTag);
	case ETriggerEvent::Started:
		return HandleInputStarted(IC, ActionData, InputTag);
	case ETriggerEvent::Ongoing:
		return HandleInputOngoing(IC, ActionData, InputTag);
	case ETriggerEvent::Canceled:
		return HandleInputCanceled(IC, ActionData, InputTag);
	case ETriggerEvent::Completed:
		return HandleInputCompleted(IC, ActionData, InputTag);
	default:
		return;
	}
}

FInputActionValue UGIPS_InputProcessor::GetInputActionValue(const FInputActionInstance& ActionData) const
{
	const FInputActionValue Value = ActionData.GetValue();
	return Value;
}

bool UGIPS_InputProcessor::CheckCanHandleInput_Implementation(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent) const
{
	return true;
}

FString UGIPS_InputProcessor::GetEditorFriendlyName_Implementation() const
{
	return TEXT("");
}

void UGIPS_InputProcessor::HandleInputCanceled_Implementation(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const
{
}

void UGIPS_InputProcessor::HandleInputCompleted_Implementation(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const
{
}

void UGIPS_InputProcessor::HandleInputOngoing_Implementation(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const
{
}

void UGIPS_InputProcessor::HandleInputStarted_Implementation(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const
{
}

void UGIPS_InputProcessor::HandleInputTriggered_Implementation(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const
{
}

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"

FString UGIPS_InputProcessor::NativeGetEditorFriendlyName() const
{
	FString BPOverride = GetEditorFriendlyName();

	return FString::Format(TEXT("Input:{0} {1}"), {UGIPS_InputFunctionLibrary::GetLastTagNameString(InputTags),BPOverride});
}

void UGIPS_InputProcessor::PreSave(FObjectPreSaveContext SaveContext)
{
	EditorFriendlyName = NativeGetEditorFriendlyName();
	UObject::PreSave(SaveContext);
}
#endif
