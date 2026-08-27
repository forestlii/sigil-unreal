// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class USigilStoryAsset;

enum class ESigilStoryPreviewConditionResult : uint8
{
	Unspecified,
	False,
	True
};

struct FSigilStoryPreviewConditionKey
{
	FName BeatId;
	int32 ConditionIndex = INDEX_NONE;

	bool operator==(const FSigilStoryPreviewConditionKey& Other) const
	{
		return BeatId == Other.BeatId && ConditionIndex == Other.ConditionIndex;
	}

	friend uint32 GetTypeHash(const FSigilStoryPreviewConditionKey& Key)
	{
		return HashCombineFast(GetTypeHash(Key.BeatId), GetTypeHash(Key.ConditionIndex));
	}
};

struct FSigilStoryPreviewEventRecord
{
	FName BeatId;
	int32 EventIndex = INDEX_NONE;
	const UClass* EventClass = nullptr;
	bool bCompleteEvent = false;
};

class FSigilStoryPreviewModel
{
public:
	bool Start(const USigilStoryAsset* InAsset, FText& OutError);
	void Invalidate();
	void SetConditionResult(
		const FSigilStoryPreviewConditionKey& Key,
		ESigilStoryPreviewConditionResult Result);
	ESigilStoryPreviewConditionResult GetConditionResult(
		const FSigilStoryPreviewConditionKey& Key) const;
	bool CanEnterBeat(FName BeatId, FText& OutReason) const;
	bool EnterBeat(FName BeatId, FText& OutError);
	bool CompleteActiveBeat(FText& OutError);

	FName GetActiveBeatId() const;
	bool IsBeatCompleted(FName BeatId) const;
	bool IsActive() const;
	const TArray<FName>& GetVisitHistory() const;
	const TArray<FSigilStoryPreviewEventRecord>& GetEventLog() const;

private:
	TWeakObjectPtr<const USigilStoryAsset> Asset;
	FName ActiveBeatId;
	TSet<FName> CompletedBeatIds;
	TMap<FSigilStoryPreviewConditionKey, ESigilStoryPreviewConditionResult> ConditionResults;
	TArray<FName> VisitHistory;
	TArray<FSigilStoryPreviewEventRecord> EventLog;
};
