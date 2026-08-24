// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilNarrativeTypes.h"
#include "UObject/Object.h"
#include "SigilDialogueSession.generated.h"

class USigilDialogueAsset;
class USigilNarrativeSubsystem;

UCLASS(BlueprintType)
class SIGILNARRATIVE_API USigilDialogueSession : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool Start(USigilDialogueAsset* InDialogueAsset, USigilNarrativeSubsystem* InNarrativeSubsystem, UObject* InContextObject);

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool Advance();

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool Choose(FName OptionId);

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	void Cancel();

	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative")
	FName GetCurrentNodeId() const;

	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative")
	bool IsActive() const;

	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative")
	bool IsCompleted() const;

private:
	bool EnterNode(FName NodeId);

	UPROPERTY()
	TObjectPtr<USigilDialogueAsset> DialogueAsset = nullptr;

	UPROPERTY()
	FSigilNarrativeContext Context;

	UPROPERTY()
	bool bActive = false;

	UPROPERTY()
	bool bCompleted = false;

	UPROPERTY()
	bool bCallbackInProgress = false;
};
