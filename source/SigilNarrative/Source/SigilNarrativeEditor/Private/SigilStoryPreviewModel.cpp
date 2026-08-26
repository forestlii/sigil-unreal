// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilStoryPreviewModel.h"

#include "SigilNarrativeCondition.h"
#include "SigilNarrativeEvent.h"
#include "SigilStoryAsset.h"

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

bool FSigilStoryPreviewModel::Start(const USigilStoryAsset* InAsset, FText& OutError)
{
	Invalidate();
	if (!InAsset || !InAsset->ValidateDefinition(OutError))
	{
		if (!InAsset)
		{
			OutError = LOCTEXT("MissingStoryPreviewAsset", "A story asset is required.");
		}
		return false;
	}

	Asset = InAsset;
	OutError = FText::GetEmpty();
	return true;
}

void FSigilStoryPreviewModel::Invalidate()
{
	Asset.Reset();
	ActiveBeatId = NAME_None;
	CompletedBeatIds.Reset();
	ConditionResults.Reset();
	VisitHistory.Reset();
	EventLog.Reset();
}

void FSigilStoryPreviewModel::SetConditionResult(
	const FSigilStoryPreviewConditionKey& Key,
	const ESigilStoryPreviewConditionResult Result)
{
	const USigilStoryAsset* StoryAsset = Asset.Get();
	const FSigilStoryBeatDefinition* Beat = StoryAsset ? StoryAsset->FindBeat(Key.BeatId) : nullptr;
	if (!Beat || !Beat->EnterConditions.IsValidIndex(Key.ConditionIndex))
	{
		return;
	}

	if (Result == ESigilStoryPreviewConditionResult::Unspecified)
	{
		ConditionResults.Remove(Key);
	}
	else
	{
		ConditionResults.Add(Key, Result);
	}
}

ESigilStoryPreviewConditionResult FSigilStoryPreviewModel::GetConditionResult(
	const FSigilStoryPreviewConditionKey& Key) const
{
	const ESigilStoryPreviewConditionResult* Result = ConditionResults.Find(Key);
	return Result ? *Result : ESigilStoryPreviewConditionResult::Unspecified;
}

bool FSigilStoryPreviewModel::CanEnterBeat(const FName BeatId, FText& OutReason) const
{
	OutReason = FText::GetEmpty();
	const USigilStoryAsset* StoryAsset = Asset.Get();
	if (!StoryAsset)
	{
		OutReason = LOCTEXT("StoryPreviewNotStarted", "Start the safe preview first.");
		return false;
	}
	if (!ActiveBeatId.IsNone())
	{
		OutReason = FText::Format(
			LOCTEXT("StoryPreviewBeatAlreadyActive", "Beat {0} is already active."),
			FText::FromName(ActiveBeatId));
		return false;
	}
	if (CompletedBeatIds.Contains(BeatId))
	{
		OutReason = FText::Format(
			LOCTEXT("StoryPreviewBeatCompleted", "Beat {0} is already completed."),
			FText::FromName(BeatId));
		return false;
	}

	const FSigilStoryBeatDefinition* Beat = StoryAsset->FindBeat(BeatId);
	if (!Beat)
	{
		OutReason = FText::Format(
			LOCTEXT("StoryPreviewBeatMissing", "Beat {0} does not exist."),
			FText::FromName(BeatId));
		return false;
	}

	for (int32 ConditionIndex = 0; ConditionIndex < Beat->EnterConditions.Num(); ++ConditionIndex)
	{
		const FSigilStoryPreviewConditionKey Key{BeatId, ConditionIndex};
		if (GetConditionResult(Key) != ESigilStoryPreviewConditionResult::True)
		{
			OutReason = FText::Format(
				LOCTEXT(
					"StoryPreviewConditionNotTrue",
					"Enter condition {0} must be set to True manually."),
				FText::AsNumber(ConditionIndex + 1));
			return false;
		}
	}
	return true;
}

bool FSigilStoryPreviewModel::EnterBeat(const FName BeatId, FText& OutError)
{
	if (!CanEnterBeat(BeatId, OutError))
	{
		return false;
	}

	const USigilStoryAsset* StoryAsset = Asset.Get();
	const FSigilStoryBeatDefinition* Beat = StoryAsset ? StoryAsset->FindBeat(BeatId) : nullptr;
	if (!Beat)
	{
		return false;
	}

	ActiveBeatId = BeatId;
	VisitHistory.Add(BeatId);
	for (int32 EventIndex = 0; EventIndex < Beat->EnterEvents.Num(); ++EventIndex)
	{
		const USigilNarrativeEvent* Event = Beat->EnterEvents[EventIndex];
		EventLog.Add({BeatId, EventIndex, Event ? Event->GetClass() : nullptr, false});
	}
	OutError = FText::GetEmpty();
	return true;
}

bool FSigilStoryPreviewModel::CompleteActiveBeat(FText& OutError)
{
	const USigilStoryAsset* StoryAsset = Asset.Get();
	if (!StoryAsset || ActiveBeatId.IsNone())
	{
		OutError = LOCTEXT("StoryPreviewNoActiveBeat", "There is no active beat to complete.");
		return false;
	}

	const FName CompletedBeatId = ActiveBeatId;
	const FSigilStoryBeatDefinition* Beat = StoryAsset->FindBeat(CompletedBeatId);
	if (!Beat)
	{
		OutError = LOCTEXT("StoryPreviewActiveBeatMissing", "The active beat no longer exists.");
		return false;
	}

	for (int32 EventIndex = 0; EventIndex < Beat->CompleteEvents.Num(); ++EventIndex)
	{
		const USigilNarrativeEvent* Event = Beat->CompleteEvents[EventIndex];
		EventLog.Add({CompletedBeatId, EventIndex, Event ? Event->GetClass() : nullptr, true});
	}
	CompletedBeatIds.Add(CompletedBeatId);
	ActiveBeatId = NAME_None;
	OutError = FText::GetEmpty();
	return true;
}

FName FSigilStoryPreviewModel::GetActiveBeatId() const
{
	return ActiveBeatId;
}

bool FSigilStoryPreviewModel::IsBeatCompleted(const FName BeatId) const
{
	return CompletedBeatIds.Contains(BeatId);
}

bool FSigilStoryPreviewModel::IsActive() const
{
	return Asset.IsValid();
}

const TArray<FName>& FSigilStoryPreviewModel::GetVisitHistory() const
{
	return VisitHistory;
}

const TArray<FSigilStoryPreviewEventRecord>& FSigilStoryPreviewModel::GetEventLog() const
{
	return EventLog;
}

#undef LOCTEXT_NAMESPACE
