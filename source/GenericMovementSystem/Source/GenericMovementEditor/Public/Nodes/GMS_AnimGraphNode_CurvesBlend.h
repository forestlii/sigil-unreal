// Copyright 2024 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "Nodes/GMS_AnimNode_CurvesBlend.h"
#include "UObject/Object.h"
#include "GMS_AnimGraphNode_CurvesBlend.generated.h"


UCLASS()
class GENERICMOVEMENTEDITOR_API UGMS_AnimGraphNode_CurvesBlend : public UAnimGraphNode_Base
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FGMS_AnimNode_CurvesBlend Node;

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;

	virtual FString GetNodeCategory() const override;
};
