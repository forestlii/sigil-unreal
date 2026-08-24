// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SigilStoryAsset.generated.h"

class USigilNarrativeCondition;
class USigilNarrativeEvent;

USTRUCT(BlueprintType)
struct SIGILNARRATIVE_API FSigilStoryBeatDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName BeatId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Sigil|Narrative")
	TArray<TObjectPtr<USigilNarrativeCondition>> EnterConditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Sigil|Narrative")
	TArray<TObjectPtr<USigilNarrativeEvent>> EnterEvents;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Sigil|Narrative")
	TArray<TObjectPtr<USigilNarrativeEvent>> CompleteEvents;
};

UCLASS(BlueprintType)
class SIGILNARRATIVE_API USigilStoryAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName StoryId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	TArray<FSigilStoryBeatDefinition> Beats;

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool ValidateDefinition(FText& OutError) const;

	const FSigilStoryBeatDefinition* FindBeat(FName BeatId) const;
};
