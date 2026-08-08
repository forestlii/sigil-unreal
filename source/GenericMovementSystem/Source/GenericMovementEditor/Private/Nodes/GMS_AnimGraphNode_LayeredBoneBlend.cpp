// Copyright 2024 RedMoonGames All Rights Reserved.

#include "Nodes/GMS_AnimGraphNode_LayeredBoneBlend.h"
#include "ToolMenus.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include "AnimGraphCommands.h"
#include "ScopedTransaction.h"

#include "Kismet2/CompilerResultsLog.h"
#include "UObject/UE5ReleaseStreamObjectVersion.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_AnimGraphNode_LayeredBoneBlend)

/////////////////////////////////////////////////////
// UGMS_AnimGraphNode_LayeredBoneBlend

#define LOCTEXT_NAMESPACE "A3Nodes"

UGMS_AnimGraphNode_LayeredBoneBlend::UGMS_AnimGraphNode_LayeredBoneBlend(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Node.AddPose();
}

FLinearColor UGMS_AnimGraphNode_LayeredBoneBlend::GetNodeTitleColor() const
{
	return FLinearColor(0.2f, 0.8f, 0.3f);
}

FText UGMS_AnimGraphNode_LayeredBoneBlend::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_LayeredBoneBlend_Tooltip", "Generic Layered blend per bone");
}

FText UGMS_AnimGraphNode_LayeredBoneBlend::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("AnimGraphNode_LayeredBoneBlend_Title", "Generic Layered blend per bone");
}

FString UGMS_AnimGraphNode_LayeredBoneBlend::GetNodeCategory() const
{
	return TEXT("Animation|Blends");
}

void UGMS_AnimGraphNode_LayeredBoneBlend::AddPinToBlendByFilter()
{
	FScopedTransaction Transaction( LOCTEXT("AddPinToBlend", "AddPinToBlendByFilter") );
	Modify();

	Node.AddPose();
	ReconstructNode();
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
}

void UGMS_AnimGraphNode_LayeredBoneBlend::RemovePinFromBlendByFilter(UEdGraphPin* Pin)
{
	FScopedTransaction Transaction( LOCTEXT("RemovePinFromBlend", "RemovePinFromBlendByFilter") );
	Modify();

	FProperty* AssociatedProperty;
	int32 ArrayIndex;
	GetPinAssociatedProperty(GetFNodeType(), Pin, /*out*/ AssociatedProperty, /*out*/ ArrayIndex);

	if (ArrayIndex != INDEX_NONE)
	{
		//@TODO: ANIMREFACTOR: Need to handle moving pins below up correctly
		// setting up removed pins info 
		RemovedPinArrayIndex = ArrayIndex;
		Node.RemovePose(ArrayIndex);
		ReconstructNode();
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
	}
}

void UGMS_AnimGraphNode_LayeredBoneBlend::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (!Context->bIsDebugging)
	{
		{
			FToolMenuSection& Section = Menu->AddSection("AnimGraphNodeLayeredBoneblend", LOCTEXT("LayeredBoneBlend", "Generic Layered Bone Blend"));
			if (Context->Pin != NULL)
			{
				// we only do this for normal BlendList/BlendList by enum, BlendList by Bool doesn't support add/remove pins
				if (Context->Pin->Direction == EGPD_Input)
				{
					//@TODO: Only offer this option on arrayed pins
					Section.AddMenuEntry(FAnimGraphCommands::Get().RemoveBlendListPin);
				}
			}
			else
			{
				Section.AddMenuEntry(FAnimGraphCommands::Get().AddBlendListPin);
			}
		}
	}
}

void UGMS_AnimGraphNode_LayeredBoneBlend::ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog)
{
	UAnimGraphNode_Base::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);

	// ensure to cache the per-bone blend weights
 	if (!Node.ArePerBoneBlendWeightsValid(ForSkeleton))
 	{
 		Node.RebuildPerBoneBlendWeights(ForSkeleton);
 	}
}


#undef LOCTEXT_NAMESPACE
