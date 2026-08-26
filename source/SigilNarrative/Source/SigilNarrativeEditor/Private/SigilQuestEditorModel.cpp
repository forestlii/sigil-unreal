// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilQuestEditorModel.h"

#include "ScopedTransaction.h"
#include "SigilNarrativeCondition.h"
#include "SigilNarrativeEvent.h"

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

FQuestStateStructOnScope::FQuestStateStructOnScope(
	USigilQuestAsset* InAsset,
	const int32 InStateIndex)
	: FStructOnScope()
	, Asset(InAsset)
	, StateIndex(InStateIndex)
{
}

uint8* FQuestStateStructOnScope::GetStructMemory()
{
	USigilQuestAsset* QuestAsset = Asset.Get();
	return QuestAsset && QuestAsset->States.IsValidIndex(StateIndex)
		? reinterpret_cast<uint8*>(&QuestAsset->States[StateIndex])
		: nullptr;
}

const uint8* FQuestStateStructOnScope::GetStructMemory() const
{
	const USigilQuestAsset* QuestAsset = Asset.Get();
	return QuestAsset && QuestAsset->States.IsValidIndex(StateIndex)
		? reinterpret_cast<const uint8*>(&QuestAsset->States[StateIndex])
		: nullptr;
}

const UScriptStruct* FQuestStateStructOnScope::GetStruct() const
{
	return FSigilQuestState::StaticStruct();
}

UPackage* FQuestStateStructOnScope::GetPackage() const
{
	return Asset.IsValid() ? Asset->GetOutermost() : nullptr;
}

void FQuestStateStructOnScope::SetPackage(UPackage* InPackage)
{
	(void)InPackage;
}

bool FQuestStateStructOnScope::IsValid() const
{
	return GetStructMemory() != nullptr;
}

FSigilQuestEditorModel::FSigilQuestEditorModel(USigilQuestAsset* InAsset)
	: Asset(InAsset)
{
}

TArray<FName> FSigilQuestEditorModel::GetFilteredStateIds(const FString& Filter) const
{
	TArray<FName> Result;
	const USigilQuestAsset* QuestAsset = Asset.Get();
	if (!QuestAsset)
	{
		return Result;
	}

	for (const FSigilQuestState& State : QuestAsset->States)
	{
		if (Filter.IsEmpty() || State.StateId.ToString().Contains(Filter, ESearchCase::IgnoreCase))
		{
			Result.Add(State.StateId);
		}
	}
	return Result;
}

int32 FSigilQuestEditorModel::FindStateIndex(const FName StateId) const
{
	const USigilQuestAsset* QuestAsset = Asset.Get();
	if (!QuestAsset)
	{
		return INDEX_NONE;
	}

	return QuestAsset->States.IndexOfByPredicate([StateId](const FSigilQuestState& State)
	{
		return State.StateId == StateId;
	});
}

FName FSigilQuestEditorModel::AddState(const ESigilQuestStateType StateType)
{
	USigilQuestAsset* QuestAsset = Asset.Get();
	if (!QuestAsset)
	{
		return NAME_None;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddQuestState", "Add Quest State"));
	QuestAsset->Modify();
	FSigilQuestState& AddedState = QuestAsset->States.AddDefaulted_GetRef();
	const TCHAR* BaseName = TEXT("State");
	if (StateType == ESigilQuestStateType::Success)
	{
		BaseName = TEXT("Success");
	}
	else if (StateType == ESigilQuestStateType::Failure)
	{
		BaseName = TEXT("Failure");
	}
	AddedState.StateId = MakeUniqueStateId(BaseName, true);
	AddedState.StateType = StateType;
	const FName AddedStateId = AddedState.StateId;
	FinishStructuralChange();
	return AddedStateId;
}

FName FSigilQuestEditorModel::DuplicateState(const FName SourceStateId)
{
	USigilQuestAsset* QuestAsset = Asset.Get();
	const int32 SourceIndex = FindStateIndex(SourceStateId);
	if (!QuestAsset || SourceIndex == INDEX_NONE)
	{
		return NAME_None;
	}

	const FScopedTransaction Transaction(LOCTEXT("DuplicateQuestState", "Duplicate Quest State"));
	QuestAsset->Modify();
	FSigilQuestState DuplicatedState = QuestAsset->States[SourceIndex];
	DuplicatedState.StateId = MakeUniqueStateId(SourceStateId.ToString() + TEXT("_Copy"), false);
	DuplicateInstancedObjects(DuplicatedState);
	const FName DuplicatedStateId = DuplicatedState.StateId;
	QuestAsset->States.Add(MoveTemp(DuplicatedState));
	FinishStructuralChange();
	return DuplicatedStateId;
}

bool FSigilQuestEditorModel::CanDeleteState(const FName StateId, FText& OutReason) const
{
	OutReason = FText::GetEmpty();
	const USigilQuestAsset* QuestAsset = Asset.Get();
	if (!QuestAsset || FindStateIndex(StateId) == INDEX_NONE)
	{
		OutReason = LOCTEXT("DeleteMissingQuestState", "The selected state no longer exists.");
		return false;
	}

	if (QuestAsset->InitialStateId == StateId)
	{
		OutReason = FText::Format(
			LOCTEXT("DeleteInitialQuestState", "State {0} is the initial quest state."),
			FText::FromName(StateId));
		return false;
	}

	for (const FSigilQuestState& SourceState : QuestAsset->States)
	{
		for (const FSigilQuestTransition& Transition : SourceState.Transitions)
		{
			if (Transition.TargetStateId == StateId)
			{
				OutReason = FText::Format(
					LOCTEXT(
						"DeleteReferencedQuestState",
						"State {0} is targeted by transition {1} on state {2}."),
					FText::FromName(StateId),
					FText::FromName(Transition.TransitionId),
					FText::FromName(SourceState.StateId));
				return false;
			}
		}
	}

	return true;
}

bool FSigilQuestEditorModel::DeleteState(const FName StateId, FText& OutReason)
{
	if (!CanDeleteState(StateId, OutReason))
	{
		return false;
	}

	USigilQuestAsset* QuestAsset = Asset.Get();
	const int32 StateIndex = FindStateIndex(StateId);
	if (!QuestAsset || StateIndex == INDEX_NONE)
	{
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteQuestState", "Delete Quest State"));
	QuestAsset->Modify();
	QuestAsset->States.RemoveAt(StateIndex);
	FinishStructuralChange();
	return true;
}

bool FSigilQuestEditorModel::SetInitialState(const FName StateId)
{
	USigilQuestAsset* QuestAsset = Asset.Get();
	if (!QuestAsset || FindStateIndex(StateId) == INDEX_NONE)
	{
		return false;
	}

	if (QuestAsset->InitialStateId == StateId)
	{
		return true;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetInitialQuestState", "Set Initial Quest State"));
	QuestAsset->Modify();
	QuestAsset->InitialStateId = StateId;
	FinishStructuralChange();
	return true;
}

bool FSigilQuestEditorModel::ReconcileStateEdit(
	const int32 StateIndex,
	const FName PreviousStateId,
	const ESigilQuestStateType PreviousStateType,
	FText& OutError)
{
	OutError = FText::GetEmpty();
	USigilQuestAsset* QuestAsset = Asset.Get();
	if (!QuestAsset || !QuestAsset->States.IsValidIndex(StateIndex))
	{
		OutError = LOCTEXT("EditedQuestStateMissing", "The edited state no longer exists.");
		return false;
	}

	FSigilQuestState& EditedState = QuestAsset->States[StateIndex];
	const FName EditedStateId = EditedState.StateId;
	const bool bDuplicateStateId = QuestAsset->States.ContainsByPredicate(
		[QuestAsset, StateIndex, EditedStateId](const FSigilQuestState& State)
		{
			return &State != &QuestAsset->States[StateIndex] && State.StateId == EditedStateId;
		});
	if (EditedStateId.IsNone() || bDuplicateStateId)
	{
		EditedState.StateId = PreviousStateId;
		OutError = EditedStateId.IsNone()
			? LOCTEXT("EmptyQuestStateId", "StateId must not be empty.")
			: FText::Format(
				LOCTEXT("DuplicateQuestStateId", "StateId {0} is already in use."),
				FText::FromName(EditedStateId));
		FinishStructuralChange();
		return false;
	}

	if (PreviousStateId != EditedStateId)
	{
		if (QuestAsset->InitialStateId == PreviousStateId)
		{
			QuestAsset->InitialStateId = EditedStateId;
		}
		for (FSigilQuestState& State : QuestAsset->States)
		{
			for (FSigilQuestTransition& Transition : State.Transitions)
			{
				if (Transition.TargetStateId == PreviousStateId)
				{
					Transition.TargetStateId = EditedStateId;
				}
			}
		}
	}

	if (PreviousStateType != EditedState.StateType
		&& EditedState.StateType != ESigilQuestStateType::Regular)
	{
		EditedState.Tasks.Reset();
		EditedState.Transitions.Reset();
	}

	FinishStructuralChange();
	return true;
}

FSimpleMulticastDelegate& FSigilQuestEditorModel::OnModelChanged()
{
	return ModelChanged;
}

USigilQuestAsset* FSigilQuestEditorModel::GetAsset() const
{
	return Asset.Get();
}

FName FSigilQuestEditorModel::MakeUniqueStateId(
	const FString& BaseName,
	const bool bUsePaddedNumber) const
{
	if (bUsePaddedNumber)
	{
		for (int32 Number = 1; Number < MAX_int32; ++Number)
		{
			const FName Candidate(*FString::Printf(TEXT("%s_%03d"), *BaseName, Number));
			if (FindStateIndex(Candidate) == INDEX_NONE)
			{
				return Candidate;
			}
		}
		return NAME_None;
	}

	const FName BaseCandidate(*BaseName);
	if (FindStateIndex(BaseCandidate) == INDEX_NONE)
	{
		return BaseCandidate;
	}
	for (int32 Number = 2; Number < MAX_int32; ++Number)
	{
		const FName Candidate(*FString::Printf(TEXT("%s_%d"), *BaseName, Number));
		if (FindStateIndex(Candidate) == INDEX_NONE)
		{
			return Candidate;
		}
	}
	return NAME_None;
}

void FSigilQuestEditorModel::DuplicateInstancedObjects(FSigilQuestState& State) const
{
	USigilQuestAsset* QuestAsset = Asset.Get();
	if (!QuestAsset)
	{
		return;
	}

	for (TObjectPtr<USigilNarrativeEvent>& EntryEvent : State.EntryEvents)
	{
		EntryEvent = EntryEvent ? DuplicateObject<USigilNarrativeEvent>(EntryEvent, QuestAsset) : nullptr;
	}
	for (FSigilQuestTransition& Transition : State.Transitions)
	{
		for (TObjectPtr<USigilNarrativeCondition>& Condition : Transition.Conditions)
		{
			Condition = Condition
				? DuplicateObject<USigilNarrativeCondition>(Condition, QuestAsset)
				: nullptr;
		}
		for (TObjectPtr<USigilNarrativeEvent>& Event : Transition.Events)
		{
			Event = Event ? DuplicateObject<USigilNarrativeEvent>(Event, QuestAsset) : nullptr;
		}
	}
}

void FSigilQuestEditorModel::FinishStructuralChange()
{
	if (USigilQuestAsset* QuestAsset = Asset.Get())
	{
		QuestAsset->MarkPackageDirty();
	}
	ModelChanged.Broadcast();
}

#undef LOCTEXT_NAMESPACE
