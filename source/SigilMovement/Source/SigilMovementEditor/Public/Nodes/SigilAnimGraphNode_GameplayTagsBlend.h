// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "AnimGraphNode_BlendListBase.h"
#include "Nodes/SigilAnimNode_GameplayTagsBlend.h"
#include "SigilAnimGraphNode_GameplayTagsBlend.generated.h"

UCLASS()
class SIGILMOVEMENTEDITOR_API USigilAnimGraphNode_GameplayTagsBlend : public UAnimGraphNode_BlendListBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FSigilAnimNode_GameplayTagsBlend Node;

public:
	USigilAnimGraphNode_GameplayTagsBlend();

	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;

	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin *> &PreviousPins) override;

	virtual FString GetNodeCategory() const override;

	virtual void CustomizePinData(UEdGraphPin *Pin, FName SourcePropertyName, int32 PinIndex) const override;

protected:
	static void GetBlendPinProperties(const UEdGraphPin *Pin, bool &bBlendPosePin, bool &bBlendTimePin);
};
