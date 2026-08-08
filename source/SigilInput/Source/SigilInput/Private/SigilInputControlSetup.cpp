// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilInputControlSetup.h"

#include "SigilInputLogChannels.h"
#include "SigilInputChecker.h"
#include "SigilInputSystemComponent.h"
#include "SigilInputFunctionLibrary.h"
#include "Misc/DataValidation.h"

void USigilInputControlSetup::HandleInput(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent) const
{
	TArray<TObjectPtr<USigilInputProcessor>> Processors = FilterInputProcessors(InputTag, TriggerEvent);
	for (int32 i = 0; i < Processors.Num(); i++)
	{
		USigilInputProcessor* Processor = Processors[i];
		if (Processor->CanHandleInput(IC, ActionData, InputTag, TriggerEvent))
		{
			Processor->HandleInput(IC, ActionData, InputTag, TriggerEvent);
			if (InputProcessorExecutionType == ESigilInputProcessorExecutionType::FirstOnly)
			{
				return;
			}
		}
		else
		{
			if (ShouldDebug(InputTag, TriggerEvent))
			{
				UE_LOG(LogSigilInput, VeryVerbose, TEXT("Input:%s can't be handled by processor:%s at index(%d), TriggerEvent:%s"), *InputTag.ToString(), *Processor->GetClass()->GetName(),
				       i, *USigilInputFunctionLibrary::GetTriggerEventString(TriggerEvent));
			}
		}
	}
}

bool USigilInputControlSetup::CheckInput(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
{
	if (InternalCheckInput(IC, ActionData, InputTag, TriggerEvent))
	{
		if (ShouldDebug(InputTag, TriggerEvent))
		{
			UE_LOG(LogSigilInput, VeryVerbose, TEXT("Input:%s passed,TriggerEvent:%s"), *InputTag.ToString(), *USigilInputFunctionLibrary::GetTriggerEventString(TriggerEvent));
		}
		IC->RegisterPassedInputEntry({InputTag, ActionData, TriggerEvent});
		return true;
	}

	if (bEnableInputBuffer)
	{
		if (IC->TrySaveInput(ActionData, InputTag, TriggerEvent))
		{
			if (ShouldDebug(InputTag, TriggerEvent))
			{
				UE_LOG(LogSigilInput, VeryVerbose, TEXT("Input:%s buffered,TriggerEvent:%s"), *InputTag.ToString(), *USigilInputFunctionLibrary::GetTriggerEventString(TriggerEvent));
			}
			IC->RegisterBufferedInputEntry({InputTag, ActionData, TriggerEvent});
			return false;
		}
	}

	if (ShouldDebug(InputTag, TriggerEvent))
	{
		UE_LOG(LogSigilInput, VeryVerbose, TEXT("Input:%s blocked,TriggerEvent:%s"), *InputTag.ToString(), *USigilInputFunctionLibrary::GetTriggerEventString(TriggerEvent));
	}

	IC->RegisterBlockedInputEntry({InputTag, ActionData, TriggerEvent});

	return false;
}

bool USigilInputControlSetup::ShouldDebug(const FGameplayTag& InputTag, const ETriggerEvent& TriggerEvent) const
{
	return bEnableInputDebug && (DebugInputTags.IsEmpty() || DebugInputTags.HasTagExact(InputTag)) && (DebugTriggerEvents.IsEmpty() || DebugTriggerEvents.Contains(TriggerEvent));
}

bool USigilInputControlSetup::InternalCheckInput(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
{
	if (InputCheckers.IsEmpty())
	{
		return true;
	}
	for (const USigilInputChecker* Checker : InputCheckers)
	{
		if (Checker == nullptr)
			continue;
		if (!Checker->CheckInput(IC, ActionData, InputTag, TriggerEvent))
			return false;
	}
	return true;
}

TArray<TObjectPtr<USigilInputProcessor>> USigilInputControlSetup::FilterInputProcessors(const FGameplayTag& InputTag, const ETriggerEvent& TriggerEvent) const
{
	return InputProcessors.FilterByPredicate([&](TObjectPtr<USigilInputProcessor> Processor)
	{
		return Processor && !Processor->InputTags.IsEmpty() && Processor->InputTags.HasTagExact(InputTag) && Processor->TriggerEvents.Contains(TriggerEvent);
	});
}

#if WITH_EDITOR
EDataValidationResult USigilInputControlSetup::IsDataValid(FDataValidationContext& Context) const
{
	for (int32 i = 0; i < InputProcessors.Num(); i++)
	{
		if (InputProcessors[i] == nullptr)
		{
			Context.AddError(FText::FromString(FString::Format(TEXT("Invalid processor at index:{0}"), {i})));
			return EDataValidationResult::Invalid;
		}
		if (InputProcessors[i]->InputTags.IsEmpty())
		{
			Context.AddWarning(FText::FromString(FString::Format(TEXT("Invalid processor at index:{0} has empty InputTags!!!"), {i})));
			return EDataValidationResult::Invalid;
		}
	}
	return Super::IsDataValid(Context);
}
#endif
