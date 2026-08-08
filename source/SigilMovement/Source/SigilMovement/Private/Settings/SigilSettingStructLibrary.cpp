// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Settings/SigilSettingStructLibrary.h"

#include "Settings/SigilSettingObjectLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilSettingStructLibrary)


#if WITH_EDITOR
void FSigilAnimDataSetting_General::PostEditChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() != GET_MEMBER_NAME_CHECKED(FSigilAnimDataSetting_General, GroundPredictionResponseChannels))
	{
		return;
	}

	GroundPredictionSweepResponses.SetAllChannels(ECR_Ignore);

	for (const auto CollisionChannel : GroundPredictionResponseChannels)
	{
		GroundPredictionSweepResponses.SetResponse(CollisionChannel, ECR_Block);
	}
}

#endif
