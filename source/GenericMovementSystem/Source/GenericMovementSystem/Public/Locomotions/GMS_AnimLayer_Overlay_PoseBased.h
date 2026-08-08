// Copyright 2024 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GMS_AnimLayer.h"
#include "GMS_AnimLayer_Overlay.h"
#include "Settings/GMS_SettingObjectLibrary.h"
#include "GMS_AnimLayer_Overlay_PoseBased.generated.h"

class UGMS_Poses_Overlay_PoseBased;

UENUM(BlueprintType)
enum class EGMS_PoseOverlaySettingType: uint8
{
	Simple,
	Layered,
	Layer_Multi
};

USTRUCT()
struct FGMS_AnimData_PoseOverlay_Simple
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> LeftArm_IdlePose{nullptr};
	
	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> RightArm_IdlePose{nullptr};

	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> LeftArm_MovePose{nullptr};
	
	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> RightArm_MovePose{nullptr};
	
	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> IdlePose{nullptr};

	UPROPERTY(EditAnywhere, Category="GMS")
	float IdlePoseExplicitTime{0};

	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> MovingPose{nullptr};

	UPROPERTY(EditAnywhere, Category="GMS")
	float MovingPoseExplicitTime{0};

	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> AimingSweepPose{nullptr};
};

USTRUCT()
struct FGMS_AnimData_PoseOverlay_Layered
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="GMS")
	FGameplayTagQuery TagQuery;
	
	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> LeftArm_IdlePose{nullptr};
	
	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> RightArm_IdlePose{nullptr};

	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> LeftArm_MovePose{nullptr};
	
	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> RightArm_MovePose{nullptr};
	
	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> IdlePose{nullptr};

	UPROPERTY(EditAnywhere, Category="GMS")
	float IdlePoseExplicitTime{0};

	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> MovingPose{nullptr};

	UPROPERTY(EditAnywhere, Category="GMS")
	float MovingPoseExplicitTime{0};

	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> AimingSweepPose{nullptr};
};

USTRUCT()
struct FGMS_PoseOverlaySetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="GMS", meta=(Categories="GMS.OverlayMode"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, Category="GMS")
	EGMS_PoseOverlaySettingType PoseOverlaySettingType{EGMS_PoseOverlaySettingType::Simple};

	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> BasePose{nullptr};

	UPROPERTY(EditAnywhere, Category="GMS", meta=(EditCondition="PoseOverlaySettingType==EGMS_PoseOverlaySettingType::Simple", EditConditionHides))
	FGMS_AnimData_PoseOverlay_Simple SimplePoseSetting;

	UPROPERTY(EditAnywhere, Category="GMS", meta=(EditCondition="PoseOverlaySettingType!=EGMS_PoseOverlaySettingType::Simple", EditConditionHides))
	TArray<FGMS_AnimData_PoseOverlay_Layered> LayeredPoseSetting;

	UPROPERTY(EditAnywhere, Category="GMS", meta=(EditCondition="PoseOverlaySettingType==EGMS_PoseOverlaySettingType::Layer_Multi", EditConditionHides))
	TObjectPtr<UGMS_Poses_Overlay_PoseBased> MultiLayeredPoseSetting;
	//TObjectPtr<UGMS_AnimLayerSetting_Overlay> AnimLayerSetting_Overlay;
};

/**
 * Native pose based overlay anim layer setting implementation.
 */
UCLASS(NotBlueprintable)
class GENERICMOVEMENTSYSTEM_API UGMS_AnimLayerSetting_Overlay_PoseBased : public UGMS_AnimLayerSetting_Overlay
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="GMS", meta=(EditCondition=false, EditConditionHides))
	TMap<FGameplayTag, FGMS_PoseOverlaySetting> AcceleratedOverlayModes;

	virtual bool IsValidForOverlayMode(const FGameplayTag& NewOverlayMode) const override;
protected:
	UPROPERTY(EditAnywhere, Category="GMS", meta=(TitleProperty="Tag"))
	TArray<FGMS_PoseOverlaySetting> OverlayModes;

#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};

/**
 * Native pose based overlay anim layer implementation.
 */
UCLASS(Abstract)
class GENERICMOVEMENTSYSTEM_API UGMS_AnimLayer_Overlay_PoseBased : public UGMS_AnimLayer
{
	GENERATED_BODY()

public:
	virtual void ApplySetting_Implementation(const UGMS_AnimLayerSetting* Setting) override;
	virtual void ResetSetting_Implementation() override;

	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> LeftArm_IdlePose{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> RightArm_IdlePose{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> LeftArm_MovePose{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> RightArm_MovePose{nullptr};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> BasePose{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> IdlePose{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float IdlePoseExplicitTime{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> MovingPose{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float MovingPoseExplicitTime{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> AimingSweepPose{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bValidAimingPose{false};

	UPROPERTY(Transient)
	TObjectPtr<const UGMS_AnimLayerSetting_Overlay_PoseBased> Settings;
};
