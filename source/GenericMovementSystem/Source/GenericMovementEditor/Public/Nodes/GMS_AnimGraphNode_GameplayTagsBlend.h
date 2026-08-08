// Copyright 2024 RedMoonGames All Rights Reserved.

#pragma once

#include "AnimGraphNode_BlendListBase.h"
#include "Nodes/GMS_AnimNode_GameplayTagsBlend.h"
#include "GMS_AnimGraphNode_GameplayTagsBlend.generated.h"

UCLASS()
class GENERICMOVEMENTEDITOR_API UGMS_AnimGraphNode_GameplayTagsBlend : public UAnimGraphNode_BlendListBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FGMS_AnimNode_GameplayTagsBlend Node;

public:
	UGMS_AnimGraphNode_GameplayTagsBlend();

	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;

	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin *> &PreviousPins) override;

	virtual FString GetNodeCategory() const override;

	virtual void CustomizePinData(UEdGraphPin *Pin, FName SourcePropertyName, int32 PinIndex) const override;

protected:
	static void GetBlendPinProperties(const UEdGraphPin *Pin, bool &bBlendPosePin, bool &bBlendTimePin);
};
