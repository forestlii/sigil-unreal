// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilNarrativeEvent.h"
#include "SigilNarrativeSetFlagEvent.generated.h"

UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class SIGILNARRATIVE_API USigilNarrativeSetFlagEvent final : public USigilNarrativeEvent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName Flag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	bool bEnabled = true;

	virtual void Execute_Implementation(const FSigilNarrativeContext& Context) override;
};
