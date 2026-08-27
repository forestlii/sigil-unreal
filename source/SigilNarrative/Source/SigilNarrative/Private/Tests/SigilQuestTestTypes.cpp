// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilQuestTestTypes.h"

#include "SigilNarrativeSubsystem.h"
#include "SigilQuestAsset.h"

void USigilNarrativeQuestTestProbe::Record(const FName Label, const FSigilNarrativeContext& Context)
{
	CallOrder.Add(Label);
	Contexts.Add(Label, Context);
}

bool USigilNarrativeQuestTestCondition::Evaluate_Implementation(const FSigilNarrativeContext& Context) const
{
	if (Probe)
	{
		Probe->Record(Label, Context);
	}

	if (Context.NarrativeSubsystem)
	{
		if (!ReentrantTaskId.IsNone() && ReentrantProgressDelta != 0)
		{
			const bool bProgressResult = Context.NarrativeSubsystem->AddQuestTaskProgress(
				Context.NarrativeId,
				ReentrantTaskId,
				ReentrantProgressDelta);
			if (Probe)
			{
				Probe->ReentrantProgressResults.Add(bProgressResult);
			}
		}

		if (!ReentrantTransitionId.IsNone())
		{
			const bool bTransitionResult = Context.NarrativeSubsystem->TryTakeQuestTransition(
				Context.NarrativeId,
				ReentrantTransitionId,
				Context.ContextObject);
			if (Probe)
			{
				Probe->ReentrantTransitionResults.Add(bTransitionResult);
			}
		}
	}

	return bResult;
}

void USigilNarrativeQuestTestEvent::Execute_Implementation(const FSigilNarrativeContext& Context)
{
	if (Probe)
	{
		Probe->Record(Label, Context);
	}

	if (!Context.NarrativeSubsystem)
	{
		return;
	}

	if (!ReentrantTaskId.IsNone() && ReentrantProgressDelta != 0)
	{
		const bool bProgressResult = Context.NarrativeSubsystem->AddQuestTaskProgress(
			Context.NarrativeId,
			ReentrantTaskId,
			ReentrantProgressDelta);
		if (Probe)
		{
			Probe->ReentrantProgressResults.Add(bProgressResult);
		}
	}

	if (!ReentrantTransitionId.IsNone())
	{
		const bool bTransitionResult = Context.NarrativeSubsystem->TryTakeQuestTransition(
			Context.NarrativeId,
			ReentrantTransitionId,
			Context.ContextObject);
		if (Probe)
		{
			Probe->ReentrantTransitionResults.Add(bTransitionResult);
		}
	}

	for (USigilQuestAsset* QuestToStart : QuestsToStart)
	{
		if (Context.NarrativeSubsystem->StartQuest(QuestToStart, Context.ContextObject) && Probe)
		{
			++Probe->StartedQuestCount;
		}
	}
}
