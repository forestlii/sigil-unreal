// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilNarrativeCondition.h"
#include "SigilNarrativeFlagCondition.generated.h"

UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class SIGILNARRATIVE_API USigilNarrativeFlagCondition final : public USigilNarrativeCondition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName Flag;

	virtual bool Evaluate_Implementation(const FSigilNarrativeContext& Context) const override;
};
