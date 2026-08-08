// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilInputProcessor.h"

#include "EnhancedInputComponent.h"
#include "SigilInputChecker.h"
#include "SigilInputFunctionLibrary.h"


USigilInputProcessor::USigilInputProcessor()
{
}

bool USigilInputProcessor::CanHandleInput(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent) const
{
	return CheckCanHandleInput(IC, ActionData, InputTag, TriggerEvent);
}

void USigilInputProcessor::HandleInput(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent) const
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

FInputActionValue USigilInputProcessor::GetInputActionValue(const FInputActionInstance& ActionData) const
{
	const FInputActionValue Value = ActionData.GetValue();
	return Value;
}

bool USigilInputProcessor::CheckCanHandleInput_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent) const
{
	return true;
}

FString USigilInputProcessor::GetEditorFriendlyName_Implementation() const
{
	return TEXT("");
}

void USigilInputProcessor::HandleInputCanceled_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const
{
}

void USigilInputProcessor::HandleInputCompleted_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const
{
}

void USigilInputProcessor::HandleInputOngoing_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const
{
}

void USigilInputProcessor::HandleInputStarted_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const
{
}

void USigilInputProcessor::HandleInputTriggered_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const
{
}

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"

FString USigilInputProcessor::NativeGetEditorFriendlyName() const
{
	FString BPOverride = GetEditorFriendlyName();

	return FString::Format(TEXT("Input:{0} {1}"), {USigilInputFunctionLibrary::GetLastTagNameString(InputTags),BPOverride});
}

void USigilInputProcessor::PreSave(FObjectPreSaveContext SaveContext)
{
	EditorFriendlyName = NativeGetEditorFriendlyName();
	UObject::PreSave(SaveContext);
}
#endif
