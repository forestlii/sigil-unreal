// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilQuestAsset.h"

#include "SigilNarrativeCondition.h"
#include "SigilNarrativeEvent.h"

bool USigilQuestAsset::ValidateDefinition(FText& OutError) const
{
	auto Fail = [&OutError](const FString& Message)
	{
		OutError = FText::FromString(Message);
		return false;
	};

	OutError = FText::GetEmpty();
	if (QuestId.IsNone())
	{
		return Fail(TEXT("QuestId must not be empty."));
	}
	if (InitialStateId.IsNone())
	{
		return Fail(TEXT("InitialStateId must not be empty."));
	}

	TSet<FName> StateIds;
	for (const FSigilQuestState& State : States)
	{
		if (State.StateId.IsNone())
		{
			return Fail(TEXT("StateId must not be empty."));
		}
		if (StateIds.Contains(State.StateId))
		{
			return Fail(FString::Printf(TEXT("Duplicate StateId: %s."), *State.StateId.ToString()));
		}
		StateIds.Add(State.StateId);

		switch (State.StateType)
		{
		case ESigilQuestStateType::Regular:
			break;

		case ESigilQuestStateType::Success:
		case ESigilQuestStateType::Failure:
			if (!State.Tasks.IsEmpty() || !State.Transitions.IsEmpty())
			{
				return Fail(FString::Printf(TEXT("Terminal state %s cannot contain tasks or transitions."), *State.StateId.ToString()));
			}
			break;

		default:
			return Fail(FString::Printf(TEXT("State %s has an unknown state type."), *State.StateId.ToString()));
		}

		for (const USigilNarrativeEvent* EntryEvent : State.EntryEvents)
		{
			if (!EntryEvent)
			{
				return Fail(FString::Printf(TEXT("State %s contains a null entry event."), *State.StateId.ToString()));
			}
		}

		TSet<FName> TaskIds;
		for (const FSigilQuestTaskDefinition& Task : State.Tasks)
		{
			if (Task.TaskId.IsNone())
			{
				return Fail(FString::Printf(TEXT("State %s contains an empty TaskId."), *State.StateId.ToString()));
			}
			if (TaskIds.Contains(Task.TaskId))
			{
				return Fail(FString::Printf(TEXT("State %s has duplicate TaskId %s."), *State.StateId.ToString(), *Task.TaskId.ToString()));
			}
			if (Task.RequiredCount <= 0)
			{
				return Fail(FString::Printf(TEXT("Task %s must have a positive RequiredCount."), *Task.TaskId.ToString()));
			}
			TaskIds.Add(Task.TaskId);
		}

		TSet<FName> TransitionIds;
		for (const FSigilQuestTransition& Transition : State.Transitions)
		{
			if (Transition.TransitionId.IsNone())
			{
				return Fail(FString::Printf(TEXT("State %s contains an empty TransitionId."), *State.StateId.ToString()));
			}
			if (TransitionIds.Contains(Transition.TransitionId))
			{
				return Fail(FString::Printf(TEXT("State %s has duplicate TransitionId %s."), *State.StateId.ToString(), *Transition.TransitionId.ToString()));
			}
			TransitionIds.Add(Transition.TransitionId);

			if (Transition.TargetStateId.IsNone())
			{
				return Fail(FString::Printf(TEXT("Transition %s must have a target state."), *Transition.TransitionId.ToString()));
			}

			TSet<FName> RequiredTaskIds;
			for (const FName RequiredTaskId : Transition.RequiredTaskIds)
			{
				if (RequiredTaskId.IsNone())
				{
					return Fail(FString::Printf(TEXT("Transition %s contains an empty required TaskId."), *Transition.TransitionId.ToString()));
				}
				if (RequiredTaskIds.Contains(RequiredTaskId))
				{
					return Fail(FString::Printf(TEXT("Transition %s repeats required TaskId %s."), *Transition.TransitionId.ToString(), *RequiredTaskId.ToString()));
				}
				if (!TaskIds.Contains(RequiredTaskId))
				{
					return Fail(FString::Printf(TEXT("Transition %s requires task %s outside its source state."), *Transition.TransitionId.ToString(), *RequiredTaskId.ToString()));
				}
				RequiredTaskIds.Add(RequiredTaskId);
			}

			for (const USigilNarrativeCondition* Condition : Transition.Conditions)
			{
				if (!Condition)
				{
					return Fail(FString::Printf(TEXT("Transition %s contains a null condition."), *Transition.TransitionId.ToString()));
				}
			}
			for (const USigilNarrativeEvent* Event : Transition.Events)
			{
				if (!Event)
				{
					return Fail(FString::Printf(TEXT("Transition %s contains a null event."), *Transition.TransitionId.ToString()));
				}
			}
		}
	}

	if (!StateIds.Contains(InitialStateId))
	{
		return Fail(TEXT("InitialStateId must reference an existing state."));
	}

	for (const FSigilQuestState& State : States)
	{
		for (const FSigilQuestTransition& Transition : State.Transitions)
		{
			if (!StateIds.Contains(Transition.TargetStateId))
			{
				return Fail(FString::Printf(TEXT("Transition %s references a missing target state."), *Transition.TransitionId.ToString()));
			}
		}
	}

	return true;
}

const FSigilQuestState* USigilQuestAsset::FindState(const FName StateId) const
{
	return States.FindByPredicate([StateId](const FSigilQuestState& State)
	{
		return State.StateId == StateId;
	});
}
