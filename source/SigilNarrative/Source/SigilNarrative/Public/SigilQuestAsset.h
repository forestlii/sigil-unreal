// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SigilQuestAsset.generated.h"

class USigilNarrativeCondition;
class USigilNarrativeEvent;

UENUM(BlueprintType)
enum class ESigilQuestStatus : uint8
{
	NotStarted,
	Active,
	Succeeded,
	Failed
};

UENUM(BlueprintType)
enum class ESigilQuestStateType : uint8
{
	Regular,
	Success,
	Failure
};

USTRUCT(BlueprintType)
struct SIGILNARRATIVE_API FSigilQuestTaskDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName TaskId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	int32 RequiredCount = 1;
};

USTRUCT(BlueprintType)
struct SIGILNARRATIVE_API FSigilQuestTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName TransitionId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName TargetStateId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	TArray<FName> RequiredTaskIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Sigil|Narrative")
	TArray<TObjectPtr<USigilNarrativeCondition>> Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Sigil|Narrative")
	TArray<TObjectPtr<USigilNarrativeEvent>> Events;
};

USTRUCT(BlueprintType)
struct SIGILNARRATIVE_API FSigilQuestState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName StateId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	ESigilQuestStateType StateType = ESigilQuestStateType::Regular;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	TArray<FSigilQuestTaskDefinition> Tasks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	TArray<FSigilQuestTransition> Transitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Sigil|Narrative")
	TArray<TObjectPtr<USigilNarrativeEvent>> EntryEvents;
};

UCLASS(BlueprintType)
class SIGILNARRATIVE_API USigilQuestAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName QuestId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName InitialStateId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	TArray<FSigilQuestState> States;

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool ValidateDefinition(FText& OutError) const;

	const FSigilQuestState* FindState(FName StateId) const;
};
