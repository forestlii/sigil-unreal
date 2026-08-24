// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilNarrativeTypes.generated.h"

class USigilNarrativeSubsystem;

USTRUCT(BlueprintType)
struct SIGILNARRATIVE_API FSigilNarrativeContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Narrative")
	TObjectPtr<USigilNarrativeSubsystem> NarrativeSubsystem = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Narrative")
	TObjectPtr<UObject> ContextObject = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Narrative")
	FName NarrativeId;

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Narrative")
	FName NodeId;
};
