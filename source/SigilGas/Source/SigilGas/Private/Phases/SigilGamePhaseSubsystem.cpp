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

UAbilitySystemComponent* USigilGamePhaseSubsystem::GetGameStateAbilitySystem(const TCHAR* Context) const
{
	const UWorld* World = GetWorld();
	AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!GameState)
	{
		UE_LOG(LogSigilGamePhase, Warning, TEXT("%s: GameState is not available yet; game phases require a GameState with an AbilitySystemComponent."), Context);
		return nullptr;
	}
	UAbilitySystemComponent* GameState_ASC = GameState->FindComponentByClass<UAbilitySystemComponent>();
	if (!GameState_ASC)
	{
		UE_LOG(LogSigilGamePhase, Warning, TEXT("%s: GameState '%s' has no AbilitySystemComponent."), Context, *GameState->GetName());
	}
	return GameState_ASC;
}

void USigilGamePhaseSubsystem::StartPhase(TSubclassOf<USigilGamePhaseAbility> PhaseAbility, FGGamePhaseDelegate PhaseEndedCallback)
{
	if (!PhaseAbility)
	{
		UE_LOG(LogSigilGamePhase, Warning, TEXT("StartPhase called with a null phase ability class."));
		PhaseEndedCallback.ExecuteIfBound(nullptr);
		return;
	}

	UAbilitySystemComponent* GameState_ASC = GetGameStateAbilitySystem(TEXT("StartPhase"));
	if (!GameState_ASC)
	{
		PhaseEndedCallback.ExecuteIfBound(nullptr);
		return;
	}

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

FDelegateHandle USigilGamePhaseSubsystem::WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, const FGGamePhaseTagDelegate& WhenPhaseActive)
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
	return WhenPhaseActive.GetHandle();
}

FDelegateHandle USigilGamePhaseSubsystem::WhenPhaseEnds(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, const FGGamePhaseTagDelegate& WhenPhaseEnd)
{
	FGPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseEnd;
	PhaseEndObservers.Add(Observer);
	return WhenPhaseEnd.GetHandle();
}

void USigilGamePhaseSubsystem::RemovePhaseObserver(FDelegateHandle Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}
	auto MatchesHandle = [Handle](const FGPhaseObserver& Observer) { return Observer.PhaseCallback.GetHandle() == Handle; };
	PhaseStartObservers.RemoveAllSwap(MatchesHandle);
	PhaseEndObservers.RemoveAllSwap(MatchesHandle);
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

	// Ending sibling phases needs CancelAbilitiesByFunc, which lives on the Sigil ASC.
	USigilAbilitySystemComponent* GameState_ASC = Cast<USigilAbilitySystemComponent>(GetGameStateAbilitySystem(TEXT("OnBeginPhase")));
	if (!GameState_ASC)
	{
		UE_LOG(LogSigilGamePhase, Warning, TEXT("OnBeginPhase: the GameState's AbilitySystemComponent must be a USigilAbilitySystemComponent for game phases to work."));
	}
	else
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
		NotifyObservers(PhaseStartObservers, IncomingPhaseTag);
	}
}

void USigilGamePhaseSubsystem::NotifyObservers(TArray<FGPhaseObserver>& Observers, const FGameplayTag& PhaseTag)
{
	// Drop observers whose bound object has been destroyed so the list cannot grow unbounded across a long session.
	Observers.RemoveAllSwap([](const FGPhaseObserver& Observer) { return !Observer.PhaseCallback.IsBound(); });

	// Iterate over a copy: a callback may register new observers.
	const TArray<FGPhaseObserver> Snapshot = Observers;
	for (const FGPhaseObserver& Observer : Snapshot)
	{
		if (Observer.IsMatch(PhaseTag))
		{
			Observer.PhaseCallback.ExecuteIfBound(PhaseTag);
		}
	}
}

void USigilGamePhaseSubsystem::OnEndPhase(const USigilGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag EndedPhaseTag = PhaseAbility->GetGamePhaseTag();
	UE_LOG(LogSigilGamePhase, Log, TEXT("Ended Phase '%s' (%s)"), *EndedPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	if (const FGGamePhaseEntry* Entry = ActivePhaseMap.Find(PhaseAbilityHandle))
	{
		Entry->PhaseEndedCallback.ExecuteIfBound(PhaseAbility);
		ActivePhaseMap.Remove(PhaseAbilityHandle);
	}

	// Notify all observers of this phase that it has ended.
	NotifyObservers(PhaseEndObservers, EndedPhaseTag);
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
