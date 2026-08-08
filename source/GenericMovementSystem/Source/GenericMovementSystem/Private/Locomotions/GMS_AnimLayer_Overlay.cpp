// Copyright 2024 RedMoonGames All Rights Reserved.


#include "Locomotions/GMS_AnimLayer_Overlay.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_AnimLayer_Overlay)


void FGMS_AnimData_Overlay::Validate()
{
#if WITH_EDITOR
	bValid = true;
	if (Sequence == nullptr)
	{
		bValid = false;
		EditorMessage = TEXT("Invalid Overlay,No valid sequence");
		return;
	}
	if (BlendMode == EGMS_LayeredBoneBlendMode::BlendMask)
	{
		if (Sequence->GetSkeleton() == nullptr || BlendMaskName == NAME_None)
		{
			bValid = false;
			EditorMessage = TEXT("Invalid Overlay,No valid blend mask name!");
			return;
		}

		UBlendProfile* BlendMask = Sequence->GetSkeleton()->GetBlendProfile(BlendMaskName);
		if (BlendMask == nullptr)
		{
			bValid = false;
			EditorMessage = TEXT("Invalid Overlay,The skeleton of animation doesn't have specified BlendMask");
			return;
		}
	}

	if (bValid)
	{
		EditorMessage = FString::Format(TEXT("Play ({0}) on ({1}) with condition({2})"), {Sequence->GetName(), BlendMaskName.ToString(), TagQuery.GetDescription()});
	}
	else
	{
		EditorMessage = TEXT("Invalid Overlay");
	}
#endif
}
