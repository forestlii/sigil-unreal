// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Locomotions/SigilAnimLayer_Overlay_PoseBased.h"

#include "Locomotions/SigilMainAnimInstance.h"
#include "Locomotions/SigilPoses_Overlay_PoseBased.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilAnimLayer_Overlay_PoseBased)

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"


void USigilAnimLayerSetting_Overlay_PoseBased::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
	AcceleratedOverlayModes.Empty();
	for (const FSigilPoseOverlaySetting& PoseOverlay : OverlayModes)
	{
		AcceleratedOverlayModes.Emplace(PoseOverlay.Tag, PoseOverlay);
	}
}
#endif


bool USigilAnimLayerSetting_Overlay_PoseBased::IsValidForOverlayMode(const FGameplayTag& NewOverlayMode) const
{
	return AcceleratedOverlayModes.Contains(NewOverlayMode) && AcceleratedOverlayModes[NewOverlayMode].BasePose != nullptr;
}


void USigilAnimLayer_Overlay_PoseBased::ApplySetting_Implementation(const USigilAnimLayerSetting* Setting)
{
	Settings = Cast<USigilAnimLayerSetting_Overlay_PoseBased>(Setting);
}

void USigilAnimLayer_Overlay_PoseBased::ResetSetting_Implementation()
{
	Settings = nullptr;
}

void USigilAnimLayer_Overlay_PoseBased::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (Settings.Get() != nullptr && GetParent() != nullptr && Settings->AcceleratedOverlayModes.Contains(GetParent()->OverlayMode))
	{
		const FSigilPoseOverlaySetting& OS = Settings->AcceleratedOverlayModes[GetParent()->OverlayMode];
		BasePose = OS.BasePose;
		if (OS.PoseOverlaySettingType == ESigilPoseOverlaySettingType::Simple)
		{
			LeftArm_IdlePose = OS.SimplePoseSetting.LeftArm_IdlePose;
			RightArm_IdlePose = OS.SimplePoseSetting.RightArm_IdlePose;
			LeftArm_MovePose = OS.SimplePoseSetting.LeftArm_MovePose;
			RightArm_MovePose = OS.SimplePoseSetting.RightArm_MovePose;
			IdlePose = OS.SimplePoseSetting.IdlePose;
			IdlePoseExplicitTime = OS.SimplePoseSetting.IdlePoseExplicitTime;
			MovingPose = OS.SimplePoseSetting.MovingPose;
			MovingPoseExplicitTime = OS.SimplePoseSetting.MovingPoseExplicitTime;
			AimingSweepPose = OS.SimplePoseSetting.AimingSweepPose;
			bValidAimingPose = OS.SimplePoseSetting.AimingSweepPose != nullptr;
		}

		if (OS.PoseOverlaySettingType != ESigilPoseOverlaySettingType::Simple) //Layer 和 Layer_Multi都需要执行这一段
		{
			bool bFound = false;
			for (const FSigilAnimData_PoseOverlay_Layered& LayeredPoseSetting : OS.LayeredPoseSetting)
			{
				if (LayeredPoseSetting.TagQuery.IsEmpty() || LayeredPoseSetting.TagQuery.Matches(GetParent()->OwnedTags))
				{
					LeftArm_IdlePose = LayeredPoseSetting.LeftArm_IdlePose;
					RightArm_IdlePose = LayeredPoseSetting.RightArm_IdlePose;
					LeftArm_MovePose = LayeredPoseSetting.LeftArm_MovePose;
					RightArm_MovePose = LayeredPoseSetting.RightArm_MovePose;
					IdlePose = LayeredPoseSetting.IdlePose;
					IdlePoseExplicitTime = LayeredPoseSetting.IdlePoseExplicitTime;
					MovingPose = LayeredPoseSetting.MovingPose;
					MovingPoseExplicitTime = LayeredPoseSetting.MovingPoseExplicitTime;
					AimingSweepPose = LayeredPoseSetting.AimingSweepPose;
					bValidAimingPose = OS.SimplePoseSetting.AimingSweepPose != nullptr;
					bFound = true;
					break;
				}
			}
			if (!bFound)
			{
				LeftArm_IdlePose = nullptr;
				RightArm_IdlePose = nullptr;
				LeftArm_MovePose = nullptr;
				RightArm_MovePose = nullptr;
				IdlePose = nullptr;
				IdlePoseExplicitTime = 0.0f;
				MovingPose = nullptr;
				MovingPoseExplicitTime = 0.0f;
				AimingSweepPose = nullptr;
				bValidAimingPose = false;
			}
		}
		if (OS.PoseOverlaySettingType == ESigilPoseOverlaySettingType::Layer_Multi)
		{
			for (const FSigilAnimData_PoseOverlay_Layered& LayeredPoseSetting : OS.MultiLayeredPoseSetting->LayeredPosesSetting)
			{
				if (LayeredPoseSetting.TagQuery.IsEmpty() || LayeredPoseSetting.TagQuery.Matches(GetParent()->OwnedTags))
				{
					LeftArm_IdlePose = LayeredPoseSetting.LeftArm_IdlePose == nullptr ? LeftArm_IdlePose : LayeredPoseSetting.LeftArm_IdlePose;
					RightArm_IdlePose = LayeredPoseSetting.RightArm_IdlePose == nullptr ? RightArm_IdlePose : LayeredPoseSetting.RightArm_IdlePose;
					LeftArm_MovePose = LayeredPoseSetting.LeftArm_MovePose == nullptr ? LeftArm_MovePose : LayeredPoseSetting.LeftArm_MovePose;
					RightArm_MovePose = LayeredPoseSetting.RightArm_MovePose == nullptr ? RightArm_MovePose : LayeredPoseSetting.RightArm_MovePose;
					IdlePose = LayeredPoseSetting.IdlePose == nullptr ? IdlePose : LayeredPoseSetting.IdlePose;
					// IdlePoseExplicitTime = LayeredPoseSetting.IdlePoseExplicitTime;
					MovingPose = LayeredPoseSetting.MovingPose == nullptr ? MovingPose : LayeredPoseSetting.MovingPose;
					// MovingPoseExplicitTime = LayeredPoseSetting.MovingPoseExplicitTime; 这三个暂时用不到
					// AimingSweepPose = LayeredPoseSetting.AimingSweepPose;
					// bValidAimingPose = OS.SimplePoseSetting.AimingSweepPose != nullptr;
					break;
				}
			}
		}
	}
	else {
		LeftArm_IdlePose = nullptr;
		RightArm_IdlePose = nullptr;
		LeftArm_MovePose = nullptr;
		RightArm_MovePose = nullptr;
		IdlePose = nullptr;
		IdlePoseExplicitTime = 0.0f;
		MovingPose = nullptr;
		MovingPoseExplicitTime = 0.0f;
		AimingSweepPose = nullptr;
		bValidAimingPose = false;
	}
}
