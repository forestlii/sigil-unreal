// Copyright 2024 RedMoonGames All Rights Reserved.


#include "Locomotions/GMS_AnimLayer_View_Default.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_AnimLayer_View_Default)


void UGMS_AnimLayer_View_Default::ApplySetting_Implementation(const UGMS_AnimLayerSetting* Setting)
{
	if (const UGMS_AnimLayerSetting_View_Default* DS = Cast<UGMS_AnimLayerSetting_View_Default>(Setting))
	{
		ResetSetting();
		BlendSpace = DS->BlendSpace;
		YawAngleOffset = DS->YawAngleOffset;
		YawAngleLimit = DS->YawAngleLimit;
		SmoothInterpSpeed = DS->SmoothInterpSpeed;
		bValidBlendSpace = BlendSpace != nullptr;
	}
}

void UGMS_AnimLayer_View_Default::ResetSetting_Implementation()
{
	bValidBlendSpace = false;
	YawAngleOffset = 0.0f;
	YawAngleLimit = FVector2D(-90.0f, 90.0f);
	SmoothInterpSpeed = 0.0f;
	BlendSpace = nullptr;
}
