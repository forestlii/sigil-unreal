// Copyright 2024 RedMoonGames All Rights Reserved.


#include "Nodes/GMS_AnimGraphNode_GameplayTagsBlend.h"

#include "GameplayTagsManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_AnimGraphNode_GameplayTagsBlend)

#define LOCTEXT_NAMESPACE "GMS_AnimGraphNode_GameplayTagsBlend"

UGMS_AnimGraphNode_GameplayTagsBlend::UGMS_AnimGraphNode_GameplayTagsBlend()
{
	Node.AddPose();
}

void UGMS_AnimGraphNode_GameplayTagsBlend::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(FGMS_AnimNode_GameplayTagsBlend, Tags))
	{
		ReconstructNode();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

FText UGMS_AnimGraphNode_GameplayTagsBlend::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Title", "Blend Poses by Gameplay Tag");
}

FText UGMS_AnimGraphNode_GameplayTagsBlend::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Blend Poses by Gameplay Tag");
}

void UGMS_AnimGraphNode_GameplayTagsBlend::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& PreviousPins)
{
	Node.RefreshPoses();

	Super::ReallocatePinsDuringReconstruction(PreviousPins);
}

FString UGMS_AnimGraphNode_GameplayTagsBlend::GetNodeCategory() const
{
	return FString{TEXTVIEW("GMS|Blends")};
}

FName GetSimpleTagName(const FGameplayTag& Tag)
{
	const auto TagNode{UGameplayTagsManager::Get().FindTagNode(Tag)};

	return TagNode.IsValid() ? TagNode->GetSimpleTagName() : NAME_None;
}

void UGMS_AnimGraphNode_GameplayTagsBlend::CustomizePinData(UEdGraphPin* Pin, const FName SourcePropertyName, const int32 PinIndex) const
{
	Super::CustomizePinData(Pin, SourcePropertyName, PinIndex);

	bool bBlendPosePin;
	bool bBlendTimePin;
	GetBlendPinProperties(Pin, bBlendPosePin, bBlendTimePin);

	if (!bBlendPosePin && !bBlendTimePin)
	{
		return;
	}

	Pin->PinFriendlyName = PinIndex <= 0
		                       ? LOCTEXT("Default", "Default")
		                       : PinIndex > Node.Tags.Num()
		                       ? LOCTEXT("Invalid", "Invalid")
		                       : FText::AsCultureInvariant(GetSimpleTagName(Node.Tags[PinIndex - 1]).ToString());

	if (bBlendPosePin)
	{
		static const FTextFormat BlendPosePinFormat{LOCTEXT("Pose", "{PinName} Pose")};

		Pin->PinFriendlyName = FText::Format(BlendPosePinFormat, {{FString{TEXTVIEW("PinName")}, Pin->PinFriendlyName}});
	}
	else if (bBlendTimePin)
	{
		static const FTextFormat BlendTimePinFormat{LOCTEXT("BlendTime", "{PinName} Blend Time")};

		Pin->PinFriendlyName = FText::Format(BlendTimePinFormat, {{FString{TEXTVIEW("PinName")}, Pin->PinFriendlyName}});
	}
}

void UGMS_AnimGraphNode_GameplayTagsBlend::GetBlendPinProperties(const UEdGraphPin* Pin, bool& bBlendPosePin, bool& bBlendTimePin)
{
	const auto PinFullName{Pin->PinName.ToString()};
	const auto SeparatorIndex{PinFullName.Find(TEXT("_"), ESearchCase::CaseSensitive)};

	if (SeparatorIndex <= 0)
	{
		bBlendPosePin = false;
		bBlendTimePin = false;
		return;
	}

	const auto PinName{PinFullName.Left(SeparatorIndex)};
	bBlendPosePin = PinName == TEXTVIEW("BlendPose");
	bBlendTimePin = PinName == TEXTVIEW("BlendTime");
}

#undef LOCTEXT_NAMESPACE
