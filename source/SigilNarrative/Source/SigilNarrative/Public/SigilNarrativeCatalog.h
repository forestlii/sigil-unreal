// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SigilNarrativeCatalog.generated.h"

class USigilQuestAsset;
class USigilStoryAsset;

UCLASS(BlueprintType)
class SIGILNARRATIVE_API USigilNarrativeCatalog : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	TArray<TObjectPtr<USigilQuestAsset>> QuestAssets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	TArray<TObjectPtr<USigilStoryAsset>> StoryAssets;

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool ValidateDefinition(FText& OutError) const;

	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative")
	USigilQuestAsset* FindQuest(FName QuestId) const;

	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative")
	USigilStoryAsset* FindStory(FName StoryId) const;
};
