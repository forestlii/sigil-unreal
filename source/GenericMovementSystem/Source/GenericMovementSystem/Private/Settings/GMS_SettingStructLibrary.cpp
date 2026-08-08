// Copyright 2024 RedMoonGames All Rights Reserved.


#include "Settings/GMS_SettingStructLibrary.h"

#include "Settings/GMS_SettingObjectLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_SettingStructLibrary)


#if WITH_EDITOR
void FGMS_AnimDataSetting_General::PostEditChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() != GET_MEMBER_NAME_CHECKED(FGMS_AnimDataSetting_General, GroundPredictionResponseChannels))
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
