// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "CombatFlow/SigilAttackResultProcessor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueFunctionLibrary.h"
#include "GameFramework/Pawn.h"
#include "SigilCombatLogChannels.h"
#include "CombatFlow/SigilAttackRequest.h"
#include "CombatFlow/SigilCombatFlow.h"
#include "UObject/ObjectSaveContext.h"
#include "Utilities/SigilGameplayCueFunctionLibrary.h"
#include "Utility/SigilCombatFunctionLibrary.h"

void USigilAttackResultProcessor::ProcessIncomingAttackResult(const FSigilAttackResult& AttackResult)
{
	if (!AttackResult.bConsumed)
	{
		HandleIncomingAttackResult(AttackResult);
	}
}

class UWorld* USigilAttackResultProcessor::GetWorld() const
{
	if (AActor* OwningActor = GetOwningActor())
	{
		return OwningActor->GetWorld();
	}
	return nullptr;
}

AActor* USigilAttackResultProcessor::GetOwningActor() const
{
	if (USigilCombatFlow* Flow = Cast<USigilCombatFlow>(GetOuter()))
	{
		return Flow->GetFlowOwner();
	}
	return nullptr;
}

UAbilitySystemComponent* USigilAttackResultProcessor::GetOwningAbilitySystemComponent() const
{
	if (AActor* OwningActor = GetOwningActor())
	{
		return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor);
	}
	return nullptr;
}

FString USigilAttackResultProcessor::GetEditorFriendlyName_Implementation() const
{
	return TEXT("");
}

void USigilAttackResultProcessor::HandleIncomingAttackResult_Implementation(const FSigilAttackResult& AttackResult) const
{
}


#if WITH_EDITORONLY_DATA
void USigilAttackResultProcessor::PreSave(FObjectPreSaveContext SaveContext)
{
	EditorFriendlyName = GetEditorFriendlyName();
	Super::PreSave(SaveContext);
}
#endif

void USigilAttackResultProcessor_WithTagRequirement::ProcessIncomingAttackResult(const FSigilAttackResult& AttackResult)
{
	if (!AttackResult.bConsumed)
	{
		bool bMatchSourceQuery = SourceTagQuery.IsEmpty() || SourceTagQuery.Matches(AttackResult.AggregatedSourceTags);
		bool bMatchTargetQuery = TargetTagQuery.IsEmpty() || TargetTagQuery.Matches(AttackResult.AggregatedTargetTags);

		if (bMatchSourceQuery && bMatchTargetQuery)
		{
			HandleIncomingAttackResult(AttackResult);
		}
	}
}

FString USigilAttackResultProcessor_WithTagRequirement::GetSourceTagQueryDesc() const
{
	return SourceTagQuery.GetDescription();
}

FString USigilAttackResultProcessor_WithTagRequirement::GetTargetTagQueryDesc() const
{
	return TargetTagQuery.GetDescription();
}

void USigilAttackResultProcessor_Death::HandleIncomingAttackResult_Implementation(const FSigilAttackResult& AttackResult) const
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (ASC)
	{
		if (true)
		{
		}
	}
	Super::HandleIncomingAttackResult_Implementation(AttackResult);
}

void USigilAttackResultProcessor_GameplayEvent::HandleIncomingAttackResult_Implementation(const FSigilAttackResult& AttackResult) const
{
	APawn* OwningPawn = Cast<APawn>(GetOwningActor());
	if (OwningPawn == nullptr)
		return;

	if (OwningPawn->HasAuthority() || OwningPawn->IsLocallyControlled())
	{
		UAbilitySystemComponent* ASC = bSendToAttacker ? AttackResult.EffectContextHandle.GetInstigatorAbilitySystemComponent() : GetOwningAbilitySystemComponent();
		if (IsValid(ASC))
		{
			FGameplayEventData Payload = FGameplayEventData();
			Payload.Instigator = AttackResult.EffectContextHandle.GetInstigator();
			// Payload.OptionalObject = AttackResult.OptionalObject;
			Payload.Target = OwningPawn;
			Payload.ContextHandle = AttackResult.EffectContextHandle;
			Payload.InstigatorTags = AttackResult.AggregatedSourceTags;
			Payload.TargetTags = AttackResult.AggregatedTargetTags;

			for (const FGameplayTag& Tag : EventTriggers)
			{
				// UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwningPawn, Tag, Payload);
				Payload.EventTag = Tag;
				// FScopedPredictionWindow NewScopedWindow(ASC, true);
				ASC->HandleGameplayEvent(Tag, &Payload);
			}
		}
	}
}

FString USigilAttackResultProcessor_GameplayEvent::GetEditorFriendlyName_Implementation() const
{
	FString Result;

	for (FGameplayTag EventTrigger : EventTriggers)
	{
		if (EventTrigger.IsValid())
		{
			TArray<FString> Parts;
			EventTrigger.GetTagName().ToString().ParseIntoArray(Parts,TEXT("."));
			if (Parts.Num() > 0)
			{
				Result.Append(FString::Format(TEXT(" ({0}) "), {Parts.Last()}));
			}
		}
	}

	return FString::Format(TEXT("Send Event:{0}"), {Result});
}

void USigilAttackResultProcessor_GameplayCue::HandleIncomingAttackResult_Implementation(const FSigilAttackResult& AttackResult) const
{
	AActor* OwningActor = GetOwningActor();
	if (!OwningActor)
	{
		return;
	}

	TArray<FGameplayTag> CuesToTrigger = GameplayCues;

	FSigilAttackDefinition Atk = USigilCombatFunctionLibrary::EffectContextGetAttackDefinition(AttackResult.EffectContextHandle);

	if (!Atk.TargetGameplayCues.IsEmpty())
	{
		CuesToTrigger = Atk.TargetGameplayCues;
	}

	FGameplayCueParameters Params = FGameplayCueParameters();
	if (const FHitResult* HitResult = AttackResult.EffectContextHandle.GetHitResult())
	{
		if (HitResult->GetActor() == OwningActor)
		{
			Params = UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult(*HitResult);
		}
	}

	Params.RawMagnitude = USigilCombatFunctionLibrary::GetTaggedValue(AttackResult.TaggedValues, RawMagnitudeTag);
	Params.NormalizedMagnitude = USigilCombatFunctionLibrary::GetTaggedValue(AttackResult.TaggedValues, NormalizedMagnitudeTag);

	Params.AggregatedSourceTags = AttackResult.AggregatedSourceTags;
	Params.AggregatedTargetTags = AttackResult.AggregatedTargetTags;
	Params.EffectCauser = AttackResult.EffectContextHandle.GetEffectCauser();
	Params.Instigator = AttackResult.EffectContextHandle.GetInstigator();

	//This is attack request.
	// Params.SourceObject = AttackResult.OptionalObject;
	
	Params.EffectContext = AttackResult.EffectContextHandle;

	ModifyGameplayCueParametersBeforeExecute(Params);

	for (const FGameplayTag& Cue : CuesToTrigger)
	{
		if (!Cue.IsValid())
			continue;;
		USigilGameplayCueFunctionLibrary::ExecuteGameplayCueLocal(OwningActor, Cue, Params);
	}
}

FString USigilAttackResultProcessor_GameplayCue::GetEditorFriendlyName_Implementation() const
{
	FString Result;
	for (FGameplayTag EventTrigger : GameplayCues)
	{
		if (EventTrigger.IsValid())
		{
			TArray<FString> Parts;
			EventTrigger.GetTagName().ToString().ParseIntoArray(Parts,TEXT("."));
			if (Parts.Num() > 0)
			{
				Result.Append(FString::Format(TEXT(" ({0}) "), {Parts.Last()}));
			}
		}
	}
	return FString::Format(TEXT("Execute cues with fallback:{0}"), {Result});
}
