// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Nodes/SigilAnimGraphNode_CurvesBlend.h"


#define LOCTEXT_NAMESPACE "SigilCurvesBlendAnimationGraphNode"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilAnimGraphNode_CurvesBlend)

FText USigilAnimGraphNode_CurvesBlend::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Title", "Blend Curves");
}

FText USigilAnimGraphNode_CurvesBlend::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Blend Curves");
}

FString USigilAnimGraphNode_CurvesBlend::GetNodeCategory() const
{
	return FString{TEXTVIEW("GMS|Blends")};
}

#undef LOCTEXT_NAMESPACE
