// Copyright 2024 RedMoonGames All Rights Reserved.


#include "Nodes/GMS_AnimNode_GameplayTagsBlend.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_AnimNode_GameplayTagsBlend)

int32 FGMS_AnimNode_GameplayTagsBlend::GetActiveChildIndex()
{
	const auto& CurrentActiveTag{GetActiveTag()};

	return CurrentActiveTag.IsValid()
			   ? GetTags().Find(CurrentActiveTag) + 1
			   : 0;
}

const FGameplayTag& FGMS_AnimNode_GameplayTagsBlend::GetActiveTag() const
{
	return GET_ANIM_NODE_DATA(FGameplayTag, ActiveTag);
}

const TArray<FGameplayTag>& FGMS_AnimNode_GameplayTagsBlend::GetTags() const
{
	return GET_ANIM_NODE_DATA(TArray<FGameplayTag>, Tags);
}

#if WITH_EDITOR
void FGMS_AnimNode_GameplayTagsBlend::RefreshPoses()
{
	const auto Difference{BlendPose.Num() - GetTags().Num() - 1};
	if (Difference == 0)
	{
		return;
	}

	if (Difference > 0)
	{
		for (auto i{Difference}; i > 0; i--)
		{
			RemovePose(BlendPose.Num() - 1);
		}
	}
	else
	{
		for (auto i{Difference}; i < 0; i++)
		{
			AddPose();
		}
	}
}
#endif
