// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Locomotions/SigilAnimLayer_View_Default.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilAnimLayer_View_Default)


void USigilAnimLayer_View_Default::ApplySetting_Implementation(const USigilAnimLayerSetting* Setting)
{
	if (const USigilAnimLayerSetting_View_Default* DS = Cast<USigilAnimLayerSetting_View_Default>(Setting))
	{
		ResetSetting();
		BlendSpace = DS->BlendSpace;
		YawAngleOffset = DS->YawAngleOffset;
		YawAngleLimit = DS->YawAngleLimit;
		SmoothInterpSpeed = DS->SmoothInterpSpeed;
		bValidBlendSpace = BlendSpace != nullptr;
	}
}

void USigilAnimLayer_View_Default::ResetSetting_Implementation()
{
	bValidBlendSpace = false;
	YawAngleOffset = 0.0f;
	YawAngleLimit = FVector2D(-90.0f, 90.0f);
	SmoothInterpSpeed = 0.0f;
	BlendSpace = nullptr;
}
