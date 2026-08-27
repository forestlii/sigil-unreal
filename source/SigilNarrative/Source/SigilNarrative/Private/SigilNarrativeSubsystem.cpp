// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilNarrativeSubsystem.h"

#include "SigilNarrativeCondition.h"
#include "SigilNarrativeEvent.h"
#include "SigilNarrativeTypes.h"

void USigilNarrativeSubsystem::BeginDialogueCallbackDispatch()
{
	++ActiveDialogueCallbackCount;
}

void USigilNarrativeSubsystem::EndDialogueCallbackDispatch()
{
	check(ActiveDialogueCallbackCount > 0);
	--ActiveDialogueCallbackCount;
}

void USigilNarrativeSubsystem::SetFlag(const FName Flag, const bool bEnabled)
{
	if (Flag.IsNone())
	{
		return;
	}

	if (bEnabled)
	{
		Flags.Add(Flag);
	}
	else
	{
		Flags.Remove(Flag);
	}
}

bool USigilNarrativeSubsystem::HasFlag(const FName Flag) const
{
	return !Flag.IsNone() && Flags.Contains(Flag);
}

bool USigilNarrativeSubsystem::StartQuest(USigilQuestAsset* QuestAsset, UObject* ContextObject)
{
	FText ValidationError;
	if (!QuestAsset || !QuestAsset->ValidateDefinition(ValidationError))
	{
		return false;
	}

	const FName QuestId = QuestAsset->QuestId;
	if (QuestStates.Contains(QuestId))
	{
		return false;
	}

	FSigilQuestRuntimeState NewRuntimeState;
	NewRuntimeState.QuestAsset = QuestAsset;
	NewRuntimeState.bCallbackInProgress = true;
	QuestStates.Add(QuestId, MoveTemp(NewRuntimeState));
	if (!EnterQuestState(QuestId, QuestAsset->InitialStateId, ContextObject))
	{
		QuestStates.Remove(QuestId);
		return false;
	}

	FSigilQuestRuntimeState* CompletedRuntimeState = QuestStates.Find(QuestId);
	if (!CompletedRuntimeState || CompletedRuntimeState->QuestAsset != QuestAsset)
	{
		return false;
	}
	CompletedRuntimeState->bCallbackInProgress = false;

	return true;
}

bool USigilNarrativeSubsystem::AddQuestTaskProgress(const FName QuestId, const FName TaskId, const int32 Delta)
{
	FSigilQuestRuntimeState* RuntimeState = QuestStates.Find(QuestId);
	if (!RuntimeState || RuntimeState->Status != ESigilQuestStatus::Active || !RuntimeState->QuestAsset || RuntimeState->bCallbackInProgress || Delta <= 0)
	{
		return false;
	}

	const FSigilQuestState* CurrentState = RuntimeState->QuestAsset->FindState(RuntimeState->CurrentStateId);
	if (!CurrentState)
	{
		return false;
	}

	const FSigilQuestTaskDefinition* Task = CurrentState->Tasks.FindByPredicate([TaskId](const FSigilQuestTaskDefinition& Candidate)
	{
		return Candidate.TaskId == TaskId;
	});
	int32* CurrentProgress = RuntimeState->TaskProgress.Find(TaskId);
	if (!Task || !CurrentProgress)
	{
		return false;
	}

	const int64 IncreasedProgress = static_cast<int64>(*CurrentProgress) + static_cast<int64>(Delta);
	*CurrentProgress = static_cast<int32>(FMath::Min<int64>(IncreasedProgress, Task->RequiredCount));
	return true;
}

bool USigilNarrativeSubsystem::TryTakeQuestTransition(
	const FName QuestId,
	const FName TransitionId,
	UObject* ContextObject)
{
	TObjectPtr<USigilQuestAsset> QuestAsset = nullptr;
	FName SourceStateId;
	FName TargetStateId;
	TArray<TObjectPtr<USigilNarrativeCondition>> Conditions;
	TArray<TObjectPtr<USigilNarrativeEvent>> Events;
	{
		FSigilQuestRuntimeState* RuntimeState = QuestStates.Find(QuestId);
		if (!RuntimeState || RuntimeState->Status != ESigilQuestStatus::Active || !RuntimeState->QuestAsset || RuntimeState->bCallbackInProgress)
		{
			return false;
		}

		QuestAsset = RuntimeState->QuestAsset;
		SourceStateId = RuntimeState->CurrentStateId;
		const FSigilQuestState* CurrentState = QuestAsset->FindState(SourceStateId);
		if (!CurrentState)
		{
			return false;
		}

		const FSigilQuestTransition* Transition = CurrentState->Transitions.FindByPredicate([TransitionId](const FSigilQuestTransition& Candidate)
		{
			return Candidate.TransitionId == TransitionId;
		});
		if (!Transition)
		{
			return false;
		}

		for (const FName RequiredTaskId : Transition->RequiredTaskIds)
		{
			const FSigilQuestTaskDefinition* Task = CurrentState->Tasks.FindByPredicate([RequiredTaskId](const FSigilQuestTaskDefinition& Candidate)
			{
				return Candidate.TaskId == RequiredTaskId;
			});
			const int32* CurrentProgress = RuntimeState->TaskProgress.Find(RequiredTaskId);
			if (!Task || !CurrentProgress || *CurrentProgress < Task->RequiredCount)
			{
				return false;
			}
		}

		TargetStateId = Transition->TargetStateId;
		Conditions = Transition->Conditions;
		Events = Transition->Events;
		RuntimeState->bCallbackInProgress = true;
	}

	FSigilNarrativeContext Context;
	Context.NarrativeSubsystem = this;
	Context.ContextObject = ContextObject;
	Context.NarrativeId = QuestId;
	Context.NodeId = SourceStateId;
	for (const USigilNarrativeCondition* Condition : Conditions)
	{
		const bool bConditionMet = Condition && Condition->IsMet(Context);
		if (!IsQuestRuntimeExpected(QuestId, QuestAsset, ESigilQuestStatus::Active, SourceStateId))
		{
			EndQuestCallbackDispatch(QuestId, QuestAsset);
			return false;
		}
		if (!bConditionMet)
		{
			EndQuestCallbackDispatch(QuestId, QuestAsset);
			return false;
		}
	}

	for (USigilNarrativeEvent* Event : Events)
	{
		if (!Event)
		{
			EndQuestCallbackDispatch(QuestId, QuestAsset);
			return false;
		}

		Event->Run(Context);
		if (!IsQuestRuntimeExpected(QuestId, QuestAsset, ESigilQuestStatus::Active, SourceStateId))
		{
			EndQuestCallbackDispatch(QuestId, QuestAsset);
			return false;
		}
	}

	const bool bEnteredTarget = EnterQuestState(QuestId, TargetStateId, ContextObject);
	EndQuestCallbackDispatch(QuestId, QuestAsset);
	return bEnteredTarget;
}

FName USigilNarrativeSubsystem::GetQuestState(const FName QuestId) const
{
	const FSigilQuestRuntimeState* RuntimeState = QuestStates.Find(QuestId);
	return RuntimeState ? RuntimeState->CurrentStateId : NAME_None;
}

ESigilQuestStatus USigilNarrativeSubsystem::GetQuestStatus(const FName QuestId) const
{
	const FSigilQuestRuntimeState* RuntimeState = QuestStates.Find(QuestId);
	return RuntimeState ? RuntimeState->Status : ESigilQuestStatus::NotStarted;
}

int32 USigilNarrativeSubsystem::GetQuestTaskProgress(const FName QuestId, const FName TaskId) const
{
	const FSigilQuestRuntimeState* RuntimeState = QuestStates.Find(QuestId);
	if (!RuntimeState)
	{
		return 0;
	}

	const int32* CurrentProgress = RuntimeState->TaskProgress.Find(TaskId);
	return CurrentProgress ? *CurrentProgress : 0;
}

bool USigilNarrativeSubsystem::EnterStoryBeat(
	USigilStoryAsset* StoryAsset,
	const FName BeatId,
	UObject* ContextObject)
{
	FText ValidationError;
	if (!StoryAsset || BeatId.IsNone() || !StoryAsset->ValidateDefinition(ValidationError))
	{
		return false;
	}

	const FName StoryId = StoryAsset->StoryId;
	const FSigilStoryBeatDefinition* Beat = StoryAsset->FindBeat(BeatId);
	if (!Beat)
	{
		return false;
	}

	TArray<TObjectPtr<USigilNarrativeCondition>> EnterConditions;
	TArray<TObjectPtr<USigilNarrativeEvent>> EnterEvents;
	bool bCreatedRuntime = false;
	{
		FSigilStoryRuntimeState* RuntimeState = StoryStates.Find(StoryId);
		if (RuntimeState)
		{
			if (RuntimeState->StoryAsset != StoryAsset
				|| RuntimeState->bCallbackInProgress
				|| !RuntimeState->ActiveBeatId.IsNone()
				|| RuntimeState->CompletedBeatIds.Contains(BeatId))
			{
				return false;
			}

			RuntimeState->bCallbackInProgress = true;
		}
		else
		{
			FSigilStoryRuntimeState NewRuntimeState;
			NewRuntimeState.StoryAsset = StoryAsset;
			NewRuntimeState.bCallbackInProgress = true;
			StoryStates.Add(StoryId, MoveTemp(NewRuntimeState));
			bCreatedRuntime = true;
		}

		EnterConditions = Beat->EnterConditions;
		EnterEvents = Beat->EnterEvents;
	}

	FSigilNarrativeContext Context;
	Context.NarrativeSubsystem = this;
	Context.ContextObject = ContextObject;
	Context.NarrativeId = StoryId;
	Context.NodeId = BeatId;
	for (const USigilNarrativeCondition* Condition : EnterConditions)
	{
		const bool bConditionMet = Condition && Condition->IsMet(Context);
		if (!IsStoryRuntimeExpected(StoryId, StoryAsset, NAME_None))
		{
			CancelStoryEnterDispatch(StoryId, StoryAsset, bCreatedRuntime);
			return false;
		}
		if (!bConditionMet)
		{
			CancelStoryEnterDispatch(StoryId, StoryAsset, bCreatedRuntime);
			return false;
		}
	}

	{
		FSigilStoryRuntimeState* RuntimeState = StoryStates.Find(StoryId);
		if (!RuntimeState
			|| RuntimeState->StoryAsset != StoryAsset
			|| !RuntimeState->bCallbackInProgress
			|| !RuntimeState->ActiveBeatId.IsNone())
		{
			CancelStoryEnterDispatch(StoryId, StoryAsset, bCreatedRuntime);
			return false;
		}

		RuntimeState->ActiveBeatId = BeatId;
	}

	for (USigilNarrativeEvent* Event : EnterEvents)
	{
		if (!Event)
		{
			EndStoryCallbackDispatch(StoryId, StoryAsset);
			return false;
		}

		Event->Run(Context);
		if (!IsStoryRuntimeExpected(StoryId, StoryAsset, BeatId))
		{
			EndStoryCallbackDispatch(StoryId, StoryAsset);
			return false;
		}
	}

	EndStoryCallbackDispatch(StoryId, StoryAsset);
	return true;
}

bool USigilNarrativeSubsystem::CompleteStoryBeat(
	const FName StoryId,
	const FName BeatId,
	UObject* ContextObject)
{
	TObjectPtr<USigilStoryAsset> StoryAsset = nullptr;
	TArray<TObjectPtr<USigilNarrativeEvent>> CompleteEvents;
	{
		FSigilStoryRuntimeState* RuntimeState = StoryStates.Find(StoryId);
		if (!RuntimeState
			|| !RuntimeState->StoryAsset
			|| RuntimeState->bCallbackInProgress
			|| RuntimeState->ActiveBeatId != BeatId
			|| RuntimeState->CompletedBeatIds.Contains(BeatId))
		{
			return false;
		}

		StoryAsset = RuntimeState->StoryAsset;
		const FSigilStoryBeatDefinition* Beat = StoryAsset->FindBeat(BeatId);
		if (!Beat)
		{
			return false;
		}

		CompleteEvents = Beat->CompleteEvents;
		RuntimeState->bCallbackInProgress = true;
	}

	FSigilNarrativeContext Context;
	Context.NarrativeSubsystem = this;
	Context.ContextObject = ContextObject;
	Context.NarrativeId = StoryId;
	Context.NodeId = BeatId;
	for (USigilNarrativeEvent* Event : CompleteEvents)
	{
		if (!Event)
		{
			EndStoryCallbackDispatch(StoryId, StoryAsset);
			return false;
		}

		Event->Run(Context);
		if (!IsStoryRuntimeExpected(StoryId, StoryAsset, BeatId))
		{
			EndStoryCallbackDispatch(StoryId, StoryAsset);
			return false;
		}
	}

	FSigilStoryRuntimeState* RuntimeState = StoryStates.Find(StoryId);
	if (!RuntimeState
		|| RuntimeState->StoryAsset != StoryAsset
		|| !RuntimeState->bCallbackInProgress
		|| RuntimeState->ActiveBeatId != BeatId)
	{
		EndStoryCallbackDispatch(StoryId, StoryAsset);
		return false;
	}

	RuntimeState->CompletedBeatIds.Add(BeatId);
	RuntimeState->ActiveBeatId = NAME_None;
	RuntimeState->bCallbackInProgress = false;
	return true;
}

FName USigilNarrativeSubsystem::GetActiveStoryBeat(const FName StoryId) const
{
	const FSigilStoryRuntimeState* RuntimeState = StoryStates.Find(StoryId);
	return RuntimeState ? RuntimeState->ActiveBeatId : NAME_None;
}

bool USigilNarrativeSubsystem::IsStoryBeatCompleted(const FName StoryId, const FName BeatId) const
{
	const FSigilStoryRuntimeState* RuntimeState = StoryStates.Find(StoryId);
	return RuntimeState && RuntimeState->CompletedBeatIds.Contains(BeatId);
}

bool USigilNarrativeSubsystem::RegisterPresentationHost(UObject* InHost)
{
	if (!InHost || !InHost->GetClass()->ImplementsInterface(USigilNarrativePresentationHost::StaticClass()))
	{
		return false;
	}

	UObject* CurrentHost = PresentationHost.Get();
	if (CurrentHost && CurrentHost != InHost)
	{
		return false;
	}

	PresentationHost = InHost;
	return true;
}

bool USigilNarrativeSubsystem::UnregisterPresentationHost(UObject* ExpectedHost)
{
	if (!ExpectedHost || PresentationHost.Get() != ExpectedHost)
	{
		return false;
	}

	if (HasActivePresentation())
	{
		CancelStoryPresentation(ActivePresentationHandle);
	}
	PresentationHost.Reset();
	return true;
}

FSigilNarrativePresentationHandle USigilNarrativeSubsystem::BeginStoryPresentation(
	const FName StoryId,
	const FName BeatId,
	UObject* ContextObject)
{
	FSigilNarrativePresentationHandle InvalidHandle;
	UObject* HostObject = PresentationHost.Get();
	if (HasActivePresentation()
		|| !HostObject
		|| !HostObject->GetClass()->ImplementsInterface(USigilNarrativePresentationHost::StaticClass()))
	{
		return InvalidHandle;
	}

	TObjectPtr<USigilStoryAsset> StoryAsset = nullptr;
	TObjectPtr<USigilNarrativePresentationAsset> Presentation = nullptr;
	{
		const FSigilStoryRuntimeState* RuntimeState = StoryStates.Find(StoryId);
		if (!RuntimeState
			|| !RuntimeState->StoryAsset
			|| RuntimeState->bCallbackInProgress
			|| RuntimeState->ActiveBeatId != BeatId)
		{
			return InvalidHandle;
		}

		StoryAsset = RuntimeState->StoryAsset;
		const FSigilStoryBeatDefinition* Beat = StoryAsset->FindBeat(BeatId);
		if (!Beat || !Beat->Presentation)
		{
			return InvalidHandle;
		}
		Presentation = Beat->Presentation;
	}

	FSigilNarrativePresentationHandle CandidateHandle;
	if (NextPresentationGeneration == MAX_int32)
	{
		return InvalidHandle;
	}
	CandidateHandle.Id = FGuid::NewGuid();
	CandidateHandle.Generation = NextPresentationGeneration + 1;

	if (!ISigilNarrativePresentationHost::Execute_CanBeginPresentation(
		HostObject, Presentation, CandidateHandle, ContextObject))
	{
		return InvalidHandle;
	}

	const FSigilStoryRuntimeState* RuntimeState = StoryStates.Find(StoryId);
	if (!RuntimeState
		|| RuntimeState->StoryAsset != StoryAsset
		|| RuntimeState->bCallbackInProgress
		|| RuntimeState->ActiveBeatId != BeatId
		|| HasActivePresentation()
		|| PresentationHost.Get() != HostObject)
	{
		return InvalidHandle;
	}

	NextPresentationGeneration = CandidateHandle.Generation;
	ActivePresentationHandle = CandidateHandle;
	ActivePresentationStoryId = StoryId;
	ActivePresentationBeatId = BeatId;
	if (!ISigilNarrativePresentationHost::Execute_BeginPresentation(
		HostObject, Presentation, CandidateHandle, ContextObject))
	{
		if (ActivePresentationHandle == CandidateHandle)
		{
			ActivePresentationHandle = {};
			ActivePresentationStoryId = NAME_None;
			ActivePresentationBeatId = NAME_None;
		}
		return InvalidHandle;
	}

	return CandidateHandle;
}

bool USigilNarrativeSubsystem::ResolveStoryPresentation(
	const FSigilNarrativePresentationHandle Handle,
	const ESigilNarrativePresentationResult Result,
	UObject* ContextObject)
{
	if (!Handle.IsValid() || ActivePresentationHandle != Handle)
	{
		return false;
	}

	const FName StoryId = ActivePresentationStoryId;
	const FName BeatId = ActivePresentationBeatId;
	ActivePresentationHandle = {};
	ActivePresentationStoryId = NAME_None;
	ActivePresentationBeatId = NAME_None;

	if (Result == ESigilNarrativePresentationResult::Completed
		|| Result == ESigilNarrativePresentationResult::Skipped)
	{
		return CompleteStoryBeat(StoryId, BeatId, ContextObject);
	}

	return true;
}

bool USigilNarrativeSubsystem::CancelStoryPresentation(
	const FSigilNarrativePresentationHandle Handle)
{
	if (!Handle.IsValid() || ActivePresentationHandle != Handle)
	{
		return false;
	}

	UObject* HostObject = PresentationHost.Get();
	ActivePresentationHandle = {};
	ActivePresentationStoryId = NAME_None;
	ActivePresentationBeatId = NAME_None;
	if (HostObject && HostObject->GetClass()->ImplementsInterface(USigilNarrativePresentationHost::StaticClass()))
	{
		ISigilNarrativePresentationHost::Execute_CancelPresentation(HostObject, Handle);
	}
	return true;
}

bool USigilNarrativeSubsystem::HasActivePresentation() const
{
	return ActivePresentationHandle.IsValid();
}

bool USigilNarrativeSubsystem::EnterQuestState(
	const FName QuestId,
	const FName StateId,
	UObject* ContextObject)
{
	TObjectPtr<USigilQuestAsset> QuestAsset = nullptr;
	ESigilQuestStatus NextStatus = ESigilQuestStatus::NotStarted;
	FName EnteredStateId;
	TArray<TObjectPtr<USigilNarrativeEvent>> EntryEvents;
	{
		FSigilQuestRuntimeState* RuntimeState = QuestStates.Find(QuestId);
		if (!RuntimeState || !RuntimeState->QuestAsset || !RuntimeState->bCallbackInProgress)
		{
			return false;
		}

		QuestAsset = RuntimeState->QuestAsset;
		const FSigilQuestState* State = QuestAsset->FindState(StateId);
		if (!State)
		{
			return false;
		}

		switch (State->StateType)
		{
		case ESigilQuestStateType::Regular:
			NextStatus = ESigilQuestStatus::Active;
			break;

		case ESigilQuestStateType::Success:
			NextStatus = ESigilQuestStatus::Succeeded;
			break;

		case ESigilQuestStateType::Failure:
			NextStatus = ESigilQuestStatus::Failed;
			break;

		default:
			return false;
		}

		EnteredStateId = State->StateId;
		EntryEvents = State->EntryEvents;
		RuntimeState->CurrentStateId = EnteredStateId;
		RuntimeState->Status = NextStatus;
		RuntimeState->TaskProgress.Reset();
		for (const FSigilQuestTaskDefinition& Task : State->Tasks)
		{
			RuntimeState->TaskProgress.Add(Task.TaskId, 0);
		}
	}

	FSigilNarrativeContext Context;
	Context.NarrativeSubsystem = this;
	Context.ContextObject = ContextObject;
	Context.NarrativeId = QuestId;
	Context.NodeId = EnteredStateId;
	for (USigilNarrativeEvent* EntryEvent : EntryEvents)
	{
		if (!EntryEvent)
		{
			return false;
		}

		EntryEvent->Run(Context);
		if (!IsQuestRuntimeExpected(QuestId, QuestAsset, NextStatus, EnteredStateId))
		{
			return false;
		}
	}

	return true;
}

bool USigilNarrativeSubsystem::IsQuestRuntimeExpected(
	const FName QuestId,
	const USigilQuestAsset* QuestAsset,
	const ESigilQuestStatus Status,
	const FName StateId) const
{
	const FSigilQuestRuntimeState* RuntimeState = QuestStates.Find(QuestId);
	return RuntimeState
		&& RuntimeState->QuestAsset == QuestAsset
		&& RuntimeState->Status == Status
		&& RuntimeState->CurrentStateId == StateId
		&& RuntimeState->bCallbackInProgress;
}

void USigilNarrativeSubsystem::EndQuestCallbackDispatch(const FName QuestId, const USigilQuestAsset* QuestAsset)
{
	FSigilQuestRuntimeState* RuntimeState = QuestStates.Find(QuestId);
	if (RuntimeState && RuntimeState->QuestAsset == QuestAsset)
	{
		RuntimeState->bCallbackInProgress = false;
	}
}

bool USigilNarrativeSubsystem::IsStoryRuntimeExpected(
	const FName StoryId,
	const USigilStoryAsset* StoryAsset,
	const FName ActiveBeatId) const
{
	const FSigilStoryRuntimeState* RuntimeState = StoryStates.Find(StoryId);
	return RuntimeState
		&& RuntimeState->StoryAsset == StoryAsset
		&& RuntimeState->ActiveBeatId == ActiveBeatId
		&& RuntimeState->bCallbackInProgress;
}

void USigilNarrativeSubsystem::EndStoryCallbackDispatch(const FName StoryId, const USigilStoryAsset* StoryAsset)
{
	FSigilStoryRuntimeState* RuntimeState = StoryStates.Find(StoryId);
	if (RuntimeState && RuntimeState->StoryAsset == StoryAsset)
	{
		RuntimeState->bCallbackInProgress = false;
	}
}

void USigilNarrativeSubsystem::CancelStoryEnterDispatch(
	const FName StoryId,
	const USigilStoryAsset* StoryAsset,
	const bool bRemovePendingRuntime)
{
	FSigilStoryRuntimeState* RuntimeState = StoryStates.Find(StoryId);
	if (!RuntimeState || RuntimeState->StoryAsset != StoryAsset || !RuntimeState->bCallbackInProgress)
	{
		return;
	}

	if (bRemovePendingRuntime && RuntimeState->ActiveBeatId.IsNone())
	{
		StoryStates.Remove(StoryId);
		return;
	}

	RuntimeState->bCallbackInProgress = false;
}
