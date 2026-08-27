// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilQuestPreviewModel.h"

#include "SigilNarrativeEvent.h"

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

bool FSigilQuestPreviewModel::Start(
	const USigilQuestAsset* InAsset,
	FText& OutError)
{
	Invalidate();
	OutError = FText::GetEmpty();
	if (!InAsset)
	{
		OutError = LOCTEXT("QuestPreviewMissingAsset", "Quest asset is unavailable.");
		return false;
	}
	if (!InAsset->ValidateDefinition(OutError))
	{
		return false;
	}

	Asset = const_cast<USigilQuestAsset*>(InAsset);
	Status = ESigilQuestStatus::Active;
	return EnterState(InAsset->InitialStateId, OutError);
}

void FSigilQuestPreviewModel::Invalidate()
{
	Asset.Reset();
	CurrentStateId = NAME_None;
	Status = ESigilQuestStatus::NotStarted;
	TaskProgress.Reset();
	ConditionResults.Reset();
	VisitHistory.Reset();
	EventLog.Reset();
}

bool FSigilQuestPreviewModel::SetTaskProgress(const FName TaskId, const int32 Progress)
{
	const USigilQuestAsset* QuestAsset = Asset.Get();
	const FSigilQuestState* State = QuestAsset ? QuestAsset->FindState(CurrentStateId) : nullptr;
	if (Status != ESigilQuestStatus::Active || !State || Progress < 0)
	{
		return false;
	}

	const FSigilQuestTaskDefinition* Task = State->Tasks.FindByPredicate(
		[TaskId](const FSigilQuestTaskDefinition& Candidate)
		{
			return Candidate.TaskId == TaskId;
		});
	if (!Task)
	{
		return false;
	}

	TaskProgress.Add(TaskId, FMath::Min(Progress, Task->RequiredCount));
	return true;
}

int32 FSigilQuestPreviewModel::GetTaskProgress(const FName TaskId) const
{
	const int32* Progress = TaskProgress.Find(TaskId);
	return Progress ? *Progress : 0;
}

void FSigilQuestPreviewModel::SetConditionResult(
	const FSigilQuestPreviewConditionKey& Key,
	const ESigilQuestPreviewConditionResult Result)
{
	if (Result == ESigilQuestPreviewConditionResult::Unspecified)
	{
		ConditionResults.Remove(Key);
		return;
	}
	ConditionResults.Add(Key, Result);
}

ESigilQuestPreviewConditionResult FSigilQuestPreviewModel::GetConditionResult(
	const FSigilQuestPreviewConditionKey& Key) const
{
	const ESigilQuestPreviewConditionResult* Result = ConditionResults.Find(Key);
	return Result ? *Result : ESigilQuestPreviewConditionResult::Unspecified;
}

bool FSigilQuestPreviewModel::CanTakeTransition(
	const FName TransitionId,
	FText& OutReason) const
{
	OutReason = FText::GetEmpty();
	const USigilQuestAsset* QuestAsset = Asset.Get();
	const FSigilQuestState* State = QuestAsset ? QuestAsset->FindState(CurrentStateId) : nullptr;
	if (Status != ESigilQuestStatus::Active || !State)
	{
		OutReason = LOCTEXT("QuestPreviewNotActive", "Start or restart the quest preview first.");
		return false;
	}

	const FSigilQuestTransition* Transition = State->Transitions.FindByPredicate(
		[TransitionId](const FSigilQuestTransition& Candidate)
		{
			return Candidate.TransitionId == TransitionId;
		});
	if (!Transition)
	{
		OutReason = LOCTEXT("QuestPreviewTransitionMissing", "The selected transition does not exist.");
		return false;
	}

	for (const FName RequiredTaskId : Transition->RequiredTaskIds)
	{
		const FSigilQuestTaskDefinition* Task = State->Tasks.FindByPredicate(
			[RequiredTaskId](const FSigilQuestTaskDefinition& Candidate)
			{
				return Candidate.TaskId == RequiredTaskId;
			});
		if (!Task || GetTaskProgress(RequiredTaskId) < Task->RequiredCount)
		{
			OutReason = FText::Format(
				LOCTEXT("QuestPreviewTaskIncomplete", "Task {0} is not complete."),
				FText::FromName(RequiredTaskId));
			return false;
		}
	}

	for (int32 ConditionIndex = 0; ConditionIndex < Transition->Conditions.Num(); ++ConditionIndex)
	{
		const FSigilQuestPreviewConditionKey Key{State->StateId, TransitionId, ConditionIndex};
		const ESigilQuestPreviewConditionResult Result = GetConditionResult(Key);
		if (Result == ESigilQuestPreviewConditionResult::Unspecified)
		{
			OutReason = FText::Format(
				LOCTEXT("QuestPreviewConditionUnspecified", "Condition {0} needs a manual preview result."),
				FText::AsNumber(ConditionIndex + 1));
			return false;
		}
		if (Result == ESigilQuestPreviewConditionResult::False)
		{
			OutReason = FText::Format(
				LOCTEXT("QuestPreviewConditionFalse", "Condition {0} is manually set to False."),
				FText::AsNumber(ConditionIndex + 1));
			return false;
		}
	}

	return true;
}

bool FSigilQuestPreviewModel::TakeTransition(
	const FName TransitionId,
	FText& OutError)
{
	if (!CanTakeTransition(TransitionId, OutError))
	{
		return false;
	}

	const USigilQuestAsset* QuestAsset = Asset.Get();
	const FSigilQuestState* State = QuestAsset ? QuestAsset->FindState(CurrentStateId) : nullptr;
	const FSigilQuestTransition* Transition = State
		? State->Transitions.FindByPredicate([TransitionId](const FSigilQuestTransition& Candidate)
		{
			return Candidate.TransitionId == TransitionId;
		})
		: nullptr;
	if (!QuestAsset || !State || !Transition || !QuestAsset->FindState(Transition->TargetStateId))
	{
		OutError = LOCTEXT("QuestPreviewChanged", "The quest definition changed. Restart the preview.");
		return false;
	}

	const FName SourceStateId = State->StateId;
	const FName TargetStateId = Transition->TargetStateId;
	for (int32 EventIndex = 0; EventIndex < Transition->Events.Num(); ++EventIndex)
	{
		const USigilNarrativeEvent* Event = Transition->Events[EventIndex];
		EventLog.Add({SourceStateId, TransitionId, EventIndex, Event ? Event->GetClass() : nullptr, false});
	}
	return EnterState(TargetStateId, OutError);
}

FName FSigilQuestPreviewModel::GetCurrentStateId() const
{
	return CurrentStateId;
}

ESigilQuestStatus FSigilQuestPreviewModel::GetStatus() const
{
	return Status;
}

bool FSigilQuestPreviewModel::IsActive() const
{
	return Status == ESigilQuestStatus::Active;
}

const TArray<FName>& FSigilQuestPreviewModel::GetVisitHistory() const
{
	return VisitHistory;
}

const TArray<FSigilQuestPreviewEventRecord>& FSigilQuestPreviewModel::GetEventLog() const
{
	return EventLog;
}

bool FSigilQuestPreviewModel::EnterState(const FName StateId, FText& OutError)
{
	const USigilQuestAsset* QuestAsset = Asset.Get();
	const FSigilQuestState* State = QuestAsset ? QuestAsset->FindState(StateId) : nullptr;
	if (!State)
	{
		OutError = LOCTEXT("QuestPreviewTargetMissing", "The target state is no longer available.");
		return false;
	}

	CurrentStateId = State->StateId;
	VisitHistory.Add(CurrentStateId);
	TaskProgress.Reset();
	for (const FSigilQuestTaskDefinition& Task : State->Tasks)
	{
		TaskProgress.Add(Task.TaskId, 0);
	}
	for (int32 EventIndex = 0; EventIndex < State->EntryEvents.Num(); ++EventIndex)
	{
		const USigilNarrativeEvent* Event = State->EntryEvents[EventIndex];
		EventLog.Add({State->StateId, NAME_None, EventIndex, Event ? Event->GetClass() : nullptr, true});
	}

	switch (State->StateType)
	{
	case ESigilQuestStateType::Success:
		Status = ESigilQuestStatus::Succeeded;
		break;
	case ESigilQuestStateType::Failure:
		Status = ESigilQuestStatus::Failed;
		break;
	case ESigilQuestStateType::Regular:
	default:
		Status = ESigilQuestStatus::Active;
		break;
	}
	OutError = FText::GetEmpty();
	return true;
}

#undef LOCTEXT_NAMESPACE
