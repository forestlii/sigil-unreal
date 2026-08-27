// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class USigilDialogueAsset;

enum class ESigilDialoguePreviewConditionResult : uint8
{
	Unspecified,
	False,
	True
};

struct FSigilDialoguePreviewConditionKey
{
	FName NodeId;
	FName OptionId;
	int32 ConditionIndex = INDEX_NONE;

	bool operator==(const FSigilDialoguePreviewConditionKey& Other) const
	{
		return NodeId == Other.NodeId
			&& OptionId == Other.OptionId
			&& ConditionIndex == Other.ConditionIndex;
	}

	friend uint32 GetTypeHash(const FSigilDialoguePreviewConditionKey& Key)
	{
		return HashCombineFast(
			HashCombineFast(GetTypeHash(Key.NodeId), GetTypeHash(Key.OptionId)),
			GetTypeHash(Key.ConditionIndex));
	}
};

struct FSigilDialoguePreviewEventRecord
{
	FName NodeId;
	FName OptionId;
	int32 EventIndex = INDEX_NONE;
	const UClass* EventClass = nullptr;
};

class FSigilDialoguePreviewModel
{
public:
	bool Start(const USigilDialogueAsset* InAsset, FText& OutError);
	void Invalidate();
	bool Advance(FText& OutError);
	bool Choose(FName OptionId, FText& OutError);
	void SetConditionResult(
		const FSigilDialoguePreviewConditionKey& Key,
		ESigilDialoguePreviewConditionResult Result);
	ESigilDialoguePreviewConditionResult GetConditionResult(
		const FSigilDialoguePreviewConditionKey& Key) const;
	bool CanChoose(FName OptionId, FText& OutReason) const;

	FName GetCurrentNodeId() const;
	bool IsActive() const;
	bool IsComplete() const;
	const TArray<FName>& GetVisitHistory() const;
	const TArray<FSigilDialoguePreviewEventRecord>& GetEventLog() const;

private:
	bool EnterNode(FName NodeId, FText& OutError);

	TWeakObjectPtr<USigilDialogueAsset> Asset;
	FName CurrentNodeId;
	bool bActive = false;
	bool bComplete = false;
	TMap<FSigilDialoguePreviewConditionKey, ESigilDialoguePreviewConditionResult> ConditionResults;
	TArray<FName> VisitHistory;
	TArray<FSigilDialoguePreviewEventRecord> EventLog;
};
