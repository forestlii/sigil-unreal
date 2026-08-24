// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SigilNarrativeTypes.h"
#include "SigilNarrativeEvent.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class SIGILNARRATIVE_API USigilNarrativeEvent : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	void Run(const FSigilNarrativeContext& Context);

	UFUNCTION(BlueprintNativeEvent, Category = "Sigil|Narrative")
	void Execute(const FSigilNarrativeContext& Context);
	virtual void Execute_Implementation(const FSigilNarrativeContext& Context);
};
