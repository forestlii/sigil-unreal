// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Phases/SigilGamePhaseSubsystem.h"
#include "Phases/SigilGamePhaseAbility.h"
#include "GameplayTagsManager.h"
#include "GameFramework/GameState.h"
#include "Engine/World.h"
#include "AbilitySystemComponent.h"
#include "Abilities/SigilGameplayAbility.h"
#include "SigilAbilitySystemComponent.h"
#include "Phases/SigilGamePhaseLog.h"

//////////////////////////////////////////////////////////////////////
// USigilGamePhaseSubsystem

USigilGamePhaseSubsystem::USigilGamePhaseSubsystem()
{
}

bool USigilGamePhaseSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (Super::ShouldCreateSubsystem(Outer))
	{
		//UWorld* World = Cast<UWorld>(Outer);
		//check(World);

		//return World->GetAuthGameMode() != nullptr;
		//return nullptr;
		return true;
	}

	return false;
}

bool USigilGamePhaseSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void USigilGamePhaseSubsystem::StartPhase(TSubclassOf<USigilGamePhaseAbility> PhaseAbility, FGGamePhaseDelegate PhaseEndedCallback)
{
	UWorld* World = GetWorld();
	UAbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<UAbilitySystemComponent>();
	if (ensure(GameState_ASC))
	{
		FGameplayAbilitySpec PhaseSpec(PhaseAbility, 1, 0, this);
		FGameplayAbilitySpecHandle SpecHandle = GameState_ASC->GiveAbilityAndActivateOnce(PhaseSpec);
		FGameplayAbilitySpec* FoundSpec = GameState_ASC->FindAbilitySpecFromHandle(SpecHandle);

		if (FoundSpec && FoundSpec->IsActive())
		{
			FGGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(SpecHandle);
			Entry.PhaseEndedCallback = PhaseEndedCallback;
		}
		else
		{
			PhaseEndedCallback.ExecuteIfBound(nullptr);
		}
	}
}

void USigilGamePhaseSubsystem::K2_StartPhase(TSubclassOf<USigilGamePhaseAbility> PhaseAbility, const FGGamePhaseDynamicDelegate& PhaseEndedDelegate)
{
	const FGGamePhaseDelegate EndedDelegate = FGGamePhaseDelegate::CreateWeakLambda(const_cast<UObject*>(PhaseEndedDelegate.GetUObject()), [PhaseEndedDelegate](const USigilGamePhaseAbility* PhaseAbility)
	{
		PhaseEndedDelegate.ExecuteIfBound(PhaseAbility);
	});

	StartPhase(PhaseAbility, EndedDelegate);
}

void USigilGamePhaseSubsystem::K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, FGGamePhaseTagDynamicDelegate WhenPhaseActive)
{
	const FGGamePhaseTagDelegate ActiveDelegate = FGGamePhaseTagDelegate::CreateWeakLambda(WhenPhaseActive.GetUObject(), [WhenPhaseActive](const FGameplayTag& PhaseTag)
	{
		WhenPhaseActive.ExecuteIfBound(PhaseTag);
	});

	WhenPhaseStartsOrIsActive(PhaseTag, MatchType, ActiveDelegate);
}

void USigilGamePhaseSubsystem::K2_WhenPhaseEnds(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, FGGamePhaseTagDynamicDelegate WhenPhaseEnd)
{
	const FGGamePhaseTagDelegate EndedDelegate = FGGamePhaseTagDelegate::CreateWeakLambda(WhenPhaseEnd.GetUObject(), [WhenPhaseEnd](const FGameplayTag& PhaseTag)
	{
		WhenPhaseEnd.ExecuteIfBound(PhaseTag);
	});

	WhenPhaseEnds(PhaseTag, MatchType, EndedDelegate);
}

void USigilGamePhaseSubsystem::WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, const FGGamePhaseTagDelegate& WhenPhaseActive)
{
	FGPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseActive;
	PhaseStartObservers.Add(Observer);

	if (IsPhaseActive(PhaseTag))
	{
		WhenPhaseActive.ExecuteIfBound(PhaseTag);
	}
}

void USigilGamePhaseSubsystem::WhenPhaseEnds(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, const FGGamePhaseTagDelegate& WhenPhaseEnd)
{
	FGPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseEnd;
	PhaseEndObservers.Add(Observer);
}

bool USigilGamePhaseSubsystem::IsPhaseActive(const FGameplayTag& PhaseTag) const
{
	for (const auto& KVP : ActivePhaseMap)
	{
		const FGGamePhaseEntry& PhaseEntry = KVP.Value;
		if (PhaseEntry.PhaseTag.MatchesTag(PhaseTag))
		{
			return true;
		}
	}

	return false;
}

void USigilGamePhaseSubsystem::OnBeginPhase(const USigilGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag IncomingPhaseTag = PhaseAbility->GetGamePhaseTag();
	const FGameplayTag IncomingPhaseParentTag = UGameplayTagsManager::Get().RequestGameplayTagDirectParent(IncomingPhaseTag);

	UE_LOG(LogSigilGamePhase, Log, TEXT("Beginning Phase '%s' (%s)"), *IncomingPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	const UWorld* World = GetWorld();
	USigilAbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<USigilAbilitySystemComponent>();
	if (ensure(GameState_ASC))
	{
		TArray<FGameplayAbilitySpec*> ActivePhases;
		for (const auto& KVP : ActivePhaseMap)
		{
			const FGameplayAbilitySpecHandle ActiveAbilityHandle = KVP.Key;
			if (FGameplayAbilitySpec* Spec = GameState_ASC->FindAbilitySpecFromHandle(ActiveAbilityHandle))
			{
				ActivePhases.Add(Spec);
			}
		}

		for (const FGameplayAbilitySpec* ActivePhase : ActivePhases)
		{
			const USigilGamePhaseAbility* ActivePhaseAbility = CastChecked<USigilGamePhaseAbility>(ActivePhase->Ability);
			const FGameplayTag ActivePhaseTag = ActivePhaseAbility->GetGamePhaseTag();

			// So if the active phase currently matches the incoming phase tag, we allow it.
			// i.e. multiple gameplay abilities can all be associated with the same phase tag.
			// For example,
			// You can be in the, Game.Playing, phase, and then start a sub-phase, like Game.Playing.SuddenDeath
			// Game.Playing phase will still be active, and if someone were to push another one, like,
			// Game.Playing.ActualSuddenDeath, it would end Game.Playing.SuddenDeath phase, but Game.Playing would
			// continue.  Similarly if we activated Game.GameOver, all the Game.Playing* phases would end.
			if (!ActivePhaseTag.MatchesTag(IncomingPhaseTag) && ActivePhaseTag.MatchesTag(IncomingPhaseParentTag))
			{
				UE_LOG(LogSigilGamePhase, Log, TEXT("\tEnding Phase '%s' (%s)"), *ActivePhaseTag.ToString(), *GetNameSafe(ActivePhaseAbility));

				FGameplayAbilitySpecHandle HandleToEnd = ActivePhase->Handle;
				GameState_ASC->CancelAbilitiesByFunc([HandleToEnd](const UGameplayAbility* AbilityToCancel, FGameplayAbilitySpecHandle Handle)
				{
					return Handle == HandleToEnd;
				}, true);
			}
		}

		FGGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(PhaseAbilityHandle);
		Entry.PhaseTag = IncomingPhaseTag;

		// Notify all observers of this phase that it has started.
		for (int32 i = 0; i < PhaseStartObservers.Num(); i++)
		{
			if (PhaseStartObservers[i].IsMatch(IncomingPhaseTag))
			{
				PhaseStartObservers[i].PhaseCallback.ExecuteIfBound(IncomingPhaseTag);
			}
		}
	}
}

void USigilGamePhaseSubsystem::OnEndPhase(const USigilGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag EndedPhaseTag = PhaseAbility->GetGamePhaseTag();
	UE_LOG(LogSigilGamePhase, Log, TEXT("Ended Phase '%s' (%s)"), *EndedPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	const FGGamePhaseEntry& Entry = ActivePhaseMap.FindChecked(PhaseAbilityHandle);
	Entry.PhaseEndedCallback.ExecuteIfBound(PhaseAbility);

	ActivePhaseMap.Remove(PhaseAbilityHandle);

	// Notify all observers of this phase that it has ended.
	for (int32 i = 0; i < PhaseEndObservers.Num(); i++)
	{
		if (PhaseEndObservers[i].IsMatch(EndedPhaseTag))
		{
			PhaseEndObservers[i].PhaseCallback.ExecuteIfBound(EndedPhaseTag);
		}
	}
}

bool USigilGamePhaseSubsystem::FGPhaseObserver::IsMatch(const FGameplayTag& ComparePhaseTag) const
{
	switch (MatchType)
	{
	case ESigilPhaseTagMatchType::ExactMatch:
		return ComparePhaseTag == PhaseTag;
	case ESigilPhaseTagMatchType::PartialMatch:
		return ComparePhaseTag.MatchesTag(PhaseTag);
	}

	return false;
}
