// Copyright 2025 RedMoonGames All Rights Reserved.


#include "CombatFlow/GCS_AttackResultProcessor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueFunctionLibrary.h"
#include "GameFramework/Pawn.h"
#include "GCS_LogChannels.h"
#include "CombatFlow/GCS_AttackRequest.h"
#include "CombatFlow/GCS_CombatFlow.h"
#include "UObject/ObjectSaveContext.h"
#include "Utilities/GGA_GameplayCueFunctionLibrary.h"
#include "Utility/GCS_CombatFunctionLibrary.h"

void UGCS_AttackResultProcessor::ProcessIncomingAttackResult(const FGCS_AttackResult& AttackResult)
{
	if (!AttackResult.bConsumed)
	{
		HandleIncomingAttackResult(AttackResult);
	}
}

class UWorld* UGCS_AttackResultProcessor::GetWorld() const
{
	if (AActor* OwningActor = GetOwningActor())
	{
		return OwningActor->GetWorld();
	}
	return nullptr;
}

AActor* UGCS_AttackResultProcessor::GetOwningActor() const
{
	if (UGCS_CombatFlow* Flow = Cast<UGCS_CombatFlow>(GetOuter()))
	{
		return Flow->GetFlowOwner();
	}
	return nullptr;
}

UAbilitySystemComponent* UGCS_AttackResultProcessor::GetOwningAbilitySystemComponent() const
{
	if (AActor* OwningActor = GetOwningActor())
	{
		return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor);
	}
	return nullptr;
}

FString UGCS_AttackResultProcessor::GetEditorFriendlyName_Implementation() const
{
	return TEXT("");
}

void UGCS_AttackResultProcessor::HandleIncomingAttackResult_Implementation(const FGCS_AttackResult& AttackResult) const
{
}


#if WITH_EDITORONLY_DATA
void UGCS_AttackResultProcessor::PreSave(FObjectPreSaveContext SaveContext)
{
	EditorFriendlyName = GetEditorFriendlyName();
	Super::PreSave(SaveContext);
}
#endif

void UGCS_AttackResultProcessor_WithTagRequirement::ProcessIncomingAttackResult(const FGCS_AttackResult& AttackResult)
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

FString UGCS_AttackResultProcessor_WithTagRequirement::GetSourceTagQueryDesc() const
{
	return SourceTagQuery.GetDescription();
}

FString UGCS_AttackResultProcessor_WithTagRequirement::GetTargetTagQueryDesc() const
{
	return TargetTagQuery.GetDescription();
}

void UGCS_AttackResultProcessor_Death::HandleIncomingAttackResult_Implementation(const FGCS_AttackResult& AttackResult) const
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

void UGCS_AttackResultProcessor_GameplayEvent::HandleIncomingAttackResult_Implementation(const FGCS_AttackResult& AttackResult) const
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

FString UGCS_AttackResultProcessor_GameplayEvent::GetEditorFriendlyName_Implementation() const
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

void UGCS_AttackResultProcessor_GameplayCue::HandleIncomingAttackResult_Implementation(const FGCS_AttackResult& AttackResult) const
{
	AActor* OwningActor = GetOwningActor();
	if (!OwningActor)
	{
		return;
	}

	TArray<FGameplayTag> CuesToTrigger = GameplayCues;

	FGCS_AttackDefinition Atk = UGCS_CombatFunctionLibrary::EffectContextGetAttackDefinition(AttackResult.EffectContextHandle);

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

	Params.RawMagnitude = UGCS_CombatFunctionLibrary::GetTaggedValue(AttackResult.TaggedValues, RawMagnitudeTag);
	Params.NormalizedMagnitude = UGCS_CombatFunctionLibrary::GetTaggedValue(AttackResult.TaggedValues, NormalizedMagnitudeTag);

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
		UGGA_GameplayCueFunctionLibrary::ExecuteGameplayCueLocal(OwningActor, Cue, Params);
	}
}

FString UGCS_AttackResultProcessor_GameplayCue::GetEditorFriendlyName_Implementation() const
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
