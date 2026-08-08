// Copyright 2024 RedMoonGames All Rights Reserved.


#include "Nodes/GMS_AnimGraphNode_CurvesBlend.h"


#define LOCTEXT_NAMESPACE "GMS_CurvesBlendAnimationGraphNode"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_AnimGraphNode_CurvesBlend)

FText UGMS_AnimGraphNode_CurvesBlend::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Title", "Blend Curves");
}

FText UGMS_AnimGraphNode_CurvesBlend::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Blend Curves");
}

FString UGMS_AnimGraphNode_CurvesBlend::GetNodeCategory() const
{
	return FString{TEXTVIEW("GMS|Blends")};
}

#undef LOCTEXT_NAMESPACE
