// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilStoryAsset.h"

#include "SigilNarrativeCondition.h"
#include "SigilNarrativeEvent.h"

bool USigilStoryAsset::ValidateDefinition(FText& OutError) const
{
	auto Fail = [&OutError](const FString& Message)
	{
		OutError = FText::FromString(Message);
		return false;
	};

	OutError = FText::GetEmpty();
	if (StoryId.IsNone())
	{
		return Fail(TEXT("StoryId must not be empty."));
	}

	TSet<FName> BeatIds;
	for (const FSigilStoryBeatDefinition& Beat : Beats)
	{
		if (Beat.BeatId.IsNone())
		{
			return Fail(TEXT("BeatId must not be empty."));
		}
		if (BeatIds.Contains(Beat.BeatId))
		{
			return Fail(FString::Printf(TEXT("Duplicate BeatId: %s."), *Beat.BeatId.ToString()));
		}
		BeatIds.Add(Beat.BeatId);

		for (const USigilNarrativeCondition* Condition : Beat.EnterConditions)
		{
			if (!Condition)
			{
				return Fail(FString::Printf(TEXT("Beat %s contains a null enter condition."), *Beat.BeatId.ToString()));
			}
		}
		for (const USigilNarrativeEvent* Event : Beat.EnterEvents)
		{
			if (!Event)
			{
				return Fail(FString::Printf(TEXT("Beat %s contains a null enter event."), *Beat.BeatId.ToString()));
			}
		}
		for (const USigilNarrativeEvent* Event : Beat.CompleteEvents)
		{
			if (!Event)
			{
				return Fail(FString::Printf(TEXT("Beat %s contains a null complete event."), *Beat.BeatId.ToString()));
			}
		}
	}

	return true;
}

const FSigilStoryBeatDefinition* USigilStoryAsset::FindBeat(const FName BeatId) const
{
	return Beats.FindByPredicate([BeatId](const FSigilStoryBeatDefinition& Beat)
	{
		return Beat.BeatId == BeatId;
	});
}
