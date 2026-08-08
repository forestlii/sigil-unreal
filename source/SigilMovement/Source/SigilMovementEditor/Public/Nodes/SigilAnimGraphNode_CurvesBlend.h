// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "Nodes/SigilAnimNode_CurvesBlend.h"
#include "UObject/Object.h"
#include "SigilAnimGraphNode_CurvesBlend.generated.h"


UCLASS()
class SIGILMOVEMENTEDITOR_API USigilAnimGraphNode_CurvesBlend : public UAnimGraphNode_Base
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FSigilAnimNode_CurvesBlend Node;

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;

	virtual FString GetNodeCategory() const override;
};
