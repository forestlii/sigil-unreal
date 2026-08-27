// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilQuestAsset.h"

class USigilQuestAsset;

enum class ESigilQuestPreviewConditionResult : uint8
{
	Unspecified,
	False,
	True
};

struct FSigilQuestPreviewConditionKey
{
	FName StateId;
	FName TransitionId;
	int32 ConditionIndex = INDEX_NONE;

	bool operator==(const FSigilQuestPreviewConditionKey& Other) const
	{
		return StateId == Other.StateId
			&& TransitionId == Other.TransitionId
			&& ConditionIndex == Other.ConditionIndex;
	}

	friend uint32 GetTypeHash(const FSigilQuestPreviewConditionKey& Key)
	{
		return HashCombineFast(
			HashCombineFast(GetTypeHash(Key.StateId), GetTypeHash(Key.TransitionId)),
			GetTypeHash(Key.ConditionIndex));
	}
};

struct FSigilQuestPreviewEventRecord
{
	FName StateId;
	FName TransitionId;
	int32 EventIndex = INDEX_NONE;
	const UClass* EventClass = nullptr;
	bool bEntryEvent = false;
};

class FSigilQuestPreviewModel
{
public:
	bool Start(const USigilQuestAsset* InAsset, FText& OutError);
	void Invalidate();
	bool SetTaskProgress(FName TaskId, int32 Progress);
	int32 GetTaskProgress(FName TaskId) const;
	void SetConditionResult(
		const FSigilQuestPreviewConditionKey& Key,
		ESigilQuestPreviewConditionResult Result);
	ESigilQuestPreviewConditionResult GetConditionResult(
		const FSigilQuestPreviewConditionKey& Key) const;
	bool CanTakeTransition(FName TransitionId, FText& OutReason) const;
	bool TakeTransition(FName TransitionId, FText& OutError);

	FName GetCurrentStateId() const;
	ESigilQuestStatus GetStatus() const;
	bool IsActive() const;
	const TArray<FName>& GetVisitHistory() const;
	const TArray<FSigilQuestPreviewEventRecord>& GetEventLog() const;

private:
	bool EnterState(FName StateId, FText& OutError);

	TWeakObjectPtr<USigilQuestAsset> Asset;
	FName CurrentStateId;
	ESigilQuestStatus Status = ESigilQuestStatus::NotStarted;
	TMap<FName, int32> TaskProgress;
	TMap<FSigilQuestPreviewConditionKey, ESigilQuestPreviewConditionResult> ConditionResults;
	TArray<FName> VisitHistory;
	TArray<FSigilQuestPreviewEventRecord> EventLog;
};
