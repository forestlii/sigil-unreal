// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilNarrativeCondition.h"
#include "SigilNarrativeEvent.h"
#include "SigilDialogueEditorTestTypes.generated.h"

UCLASS()
class USigilDialogueEditorTestCondition final : public USigilNarrativeCondition
{
	GENERATED_BODY()

public:
	static int32 CallCount;

	virtual bool Evaluate_Implementation(const FSigilNarrativeContext& Context) const override;
};

UCLASS()
class USigilDialogueEditorTestEvent final : public USigilNarrativeEvent
{
	GENERATED_BODY()

public:
	static int32 CallCount;

	virtual void Execute_Implementation(const FSigilNarrativeContext& Context) override;
};
