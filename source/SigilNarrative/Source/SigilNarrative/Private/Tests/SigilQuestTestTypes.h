// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilNarrativeCondition.h"
#include "SigilNarrativeEvent.h"
#include "SigilNarrativeTypes.h"
#include "SigilQuestTestTypes.generated.h"

class USigilQuestAsset;

UCLASS()
class USigilNarrativeQuestTestProbe final : public UObject
{
	GENERATED_BODY()

public:
	void Record(FName Label, const FSigilNarrativeContext& Context);

	UPROPERTY()
	TArray<FName> CallOrder;

	UPROPERTY()
	TMap<FName, FSigilNarrativeContext> Contexts;

	UPROPERTY()
	TArray<bool> ReentrantTransitionResults;

	UPROPERTY()
	TArray<bool> ReentrantProgressResults;

	UPROPERTY()
	int32 StartedQuestCount = 0;
};

UCLASS()
class USigilNarrativeQuestTestCondition final : public USigilNarrativeCondition
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<USigilNarrativeQuestTestProbe> Probe = nullptr;

	UPROPERTY()
	FName Label;

	UPROPERTY()
	bool bResult = true;

	UPROPERTY()
	FName ReentrantTransitionId;

	UPROPERTY()
	FName ReentrantTaskId;

	UPROPERTY()
	int32 ReentrantProgressDelta = 0;

	virtual bool Evaluate_Implementation(const FSigilNarrativeContext& Context) const override;
};

UCLASS()
class USigilNarrativeQuestTestEvent final : public USigilNarrativeEvent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<USigilNarrativeQuestTestProbe> Probe = nullptr;

	UPROPERTY()
	FName Label;

	UPROPERTY()
	FName ReentrantTransitionId;

	UPROPERTY()
	FName ReentrantTaskId;

	UPROPERTY()
	int32 ReentrantProgressDelta = 0;

	UPROPERTY()
	TArray<TObjectPtr<USigilQuestAsset>> QuestsToStart;

	virtual void Execute_Implementation(const FSigilNarrativeContext& Context) override;
};
