// Copyright 2024 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AnimGraphNode_BlendListBase.h"
#include "Nodes/GMS_AnimNode_LayeredBoneBlend.h"
#include "GMS_AnimGraphNode_LayeredBoneBlend.generated.h"

UCLASS(MinimalAPI)
class UGMS_AnimGraphNode_LayeredBoneBlend : public UAnimGraphNode_BlendListBase
{
	GENERATED_UCLASS_BODY()
	UPROPERTY(EditAnywhere, Category=Settings)
	FGMS_AnimNode_LayeredBoneBlend Node;

	// Adds a new pose pin
	//@TODO: Generalize this behavior (returning a list of actions/delegates maybe?)
	GENERICMOVEMENTEDITOR_API virtual void AddPinToBlendByFilter();
	GENERICMOVEMENTEDITOR_API virtual void RemovePinFromBlendByFilter(UEdGraphPin* Pin);

	// UObject interface
	// End of UObject interface

	// UEdGraphNode interface
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	// End of UEdGraphNode interface

	// UK2Node interface
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	// End of UK2Node interface

	// UAnimGraphNode_Base interface
	virtual FString GetNodeCategory() const override;
	// End of UAnimGraphNode_Base interface


	// Gives each visual node a chance to validate that they are still valid in the context of the compiled class, giving a last shot at error or warning generation after primary compilation is finished
	virtual void ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog) override;
	// End of UAnimGraphNode_Base interface
};
