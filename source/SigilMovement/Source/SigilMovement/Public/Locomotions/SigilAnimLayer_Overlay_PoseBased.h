// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilAnimLayer.h"
#include "SigilAnimLayer_Overlay.h"
#include "Settings/SigilSettingObjectLibrary.h"
#include "SigilAnimLayer_Overlay_PoseBased.generated.h"

class USigilPoses_Overlay_PoseBased;

UENUM(BlueprintType)
enum class ESigilPoseOverlaySettingType: uint8
{
	Simple,
	Layered,
	Layer_Multi
};

USTRUCT()
struct FSigilAnimData_PoseOverlay_Simple
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
struct FSigilAnimData_PoseOverlay_Layered
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
struct FSigilPoseOverlaySetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="GMS", meta=(Categories="Sigil.Movement.OverlayMode"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, Category="GMS")
	ESigilPoseOverlaySettingType PoseOverlaySettingType{ESigilPoseOverlaySettingType::Simple};

	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> BasePose{nullptr};

	UPROPERTY(EditAnywhere, Category="GMS", meta=(EditCondition="PoseOverlaySettingType==ESigilPoseOverlaySettingType::Simple", EditConditionHides))
	FSigilAnimData_PoseOverlay_Simple SimplePoseSetting;

	UPROPERTY(EditAnywhere, Category="GMS", meta=(EditCondition="PoseOverlaySettingType!=ESigilPoseOverlaySettingType::Simple", EditConditionHides))
	TArray<FSigilAnimData_PoseOverlay_Layered> LayeredPoseSetting;

	UPROPERTY(EditAnywhere, Category="GMS", meta=(EditCondition="PoseOverlaySettingType==ESigilPoseOverlaySettingType::Layer_Multi", EditConditionHides))
	TObjectPtr<USigilPoses_Overlay_PoseBased> MultiLayeredPoseSetting;
	//TObjectPtr<USigilAnimLayerSetting_Overlay> AnimLayerSetting_Overlay;
};

/**
 * Native pose based overlay anim layer setting implementation.
 */
UCLASS(NotBlueprintable)
class SIGILMOVEMENT_API USigilAnimLayerSetting_Overlay_PoseBased : public USigilAnimLayerSetting_Overlay
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="GMS", meta=(EditCondition=false, EditConditionHides))
	TMap<FGameplayTag, FSigilPoseOverlaySetting> AcceleratedOverlayModes;

	virtual bool IsValidForOverlayMode(const FGameplayTag& NewOverlayMode) const override;
protected:
	UPROPERTY(EditAnywhere, Category="GMS", meta=(TitleProperty="Tag"))
	TArray<FSigilPoseOverlaySetting> OverlayModes;

#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};

/**
 * Native pose based overlay anim layer implementation.
 */
UCLASS(Abstract)
class SIGILMOVEMENT_API USigilAnimLayer_Overlay_PoseBased : public USigilAnimLayer
{
	GENERATED_BODY()

public:
	virtual void ApplySetting_Implementation(const USigilAnimLayerSetting* Setting) override;
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
	TObjectPtr<const USigilAnimLayerSetting_Overlay_PoseBased> Settings;
};
