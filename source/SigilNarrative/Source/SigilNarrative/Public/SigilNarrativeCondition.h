// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SigilNarrativeTypes.h"
#include "SigilNarrativeCondition.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, Const)
class SIGILNARRATIVE_API USigilNarrativeCondition : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	bool bNegate = false;

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool IsMet(const FSigilNarrativeContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Sigil|Narrative")
	bool Evaluate(const FSigilNarrativeContext& Context) const;
	virtual bool Evaluate_Implementation(const FSigilNarrativeContext& Context) const;
};
