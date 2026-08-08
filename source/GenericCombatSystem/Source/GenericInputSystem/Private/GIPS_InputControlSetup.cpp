// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GIPS_InputControlSetup.h"

#include "GIPS_LogChannels.h"
#include "GIPS_InputChecker.h"
#include "GIPS_InputSystemComponent.h"
#include "GIPS_InputFunctionLibrary.h"
#include "Misc/DataValidation.h"

void UGIPS_InputControlSetup::HandleInput(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent) const
{
	TArray<TObjectPtr<UGIPS_InputProcessor>> Processors = FilterInputProcessors(InputTag, TriggerEvent);
	for (int32 i = 0; i < Processors.Num(); i++)
	{
		UGIPS_InputProcessor* Processor = Processors[i];
		if (Processor->CanHandleInput(IC, ActionData, InputTag, TriggerEvent))
		{
			Processor->HandleInput(IC, ActionData, InputTag, TriggerEvent);
			if (InputProcessorExecutionType == EGIPS_InputProcessorExecutionType::FirstOnly)
			{
				return;
			}
		}
		else
		{
			if (ShouldDebug(InputTag, TriggerEvent))
			{
				UE_LOG(LogGIPS, VeryVerbose, TEXT("Input:%s can't be handled by processor:%s at index(%d), TriggerEvent:%s"), *InputTag.ToString(), *Processor->GetClass()->GetName(),
				       i, *UGIPS_InputFunctionLibrary::GetTriggerEventString(TriggerEvent));
			}
		}
	}
}

bool UGIPS_InputControlSetup::CheckInput(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
{
	if (InternalCheckInput(IC, ActionData, InputTag, TriggerEvent))
	{
		if (ShouldDebug(InputTag, TriggerEvent))
		{
			UE_LOG(LogGIPS, VeryVerbose, TEXT("Input:%s passed,TriggerEvent:%s"), *InputTag.ToString(), *UGIPS_InputFunctionLibrary::GetTriggerEventString(TriggerEvent));
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
				UE_LOG(LogGIPS, VeryVerbose, TEXT("Input:%s buffered,TriggerEvent:%s"), *InputTag.ToString(), *UGIPS_InputFunctionLibrary::GetTriggerEventString(TriggerEvent));
			}
			IC->RegisterBufferedInputEntry({InputTag, ActionData, TriggerEvent});
			return false;
		}
	}

	if (ShouldDebug(InputTag, TriggerEvent))
	{
		UE_LOG(LogGIPS, VeryVerbose, TEXT("Input:%s blocked,TriggerEvent:%s"), *InputTag.ToString(), *UGIPS_InputFunctionLibrary::GetTriggerEventString(TriggerEvent));
	}

	IC->RegisterBlockedInputEntry({InputTag, ActionData, TriggerEvent});

	return false;
}

bool UGIPS_InputControlSetup::ShouldDebug(const FGameplayTag& InputTag, const ETriggerEvent& TriggerEvent) const
{
	return bEnableInputDebug && (DebugInputTags.IsEmpty() || DebugInputTags.HasTagExact(InputTag)) && (DebugTriggerEvents.IsEmpty() || DebugTriggerEvents.Contains(TriggerEvent));
}

bool UGIPS_InputControlSetup::InternalCheckInput(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
{
	if (InputCheckers.IsEmpty())
	{
		return true;
	}
	for (const UGIPS_InputChecker* Checker : InputCheckers)
	{
		if (Checker == nullptr)
			continue;
		if (!Checker->CheckInput(IC, ActionData, InputTag, TriggerEvent))
			return false;
	}
	return true;
}

TArray<TObjectPtr<UGIPS_InputProcessor>> UGIPS_InputControlSetup::FilterInputProcessors(const FGameplayTag& InputTag, const ETriggerEvent& TriggerEvent) const
{
	return InputProcessors.FilterByPredicate([&](TObjectPtr<UGIPS_InputProcessor> Processor)
	{
		return Processor && !Processor->InputTags.IsEmpty() && Processor->InputTags.HasTagExact(InputTag) && Processor->TriggerEvents.Contains(TriggerEvent);
	});
}

#if WITH_EDITOR
EDataValidationResult UGIPS_InputControlSetup::IsDataValid(FDataValidationContext& Context) const
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
