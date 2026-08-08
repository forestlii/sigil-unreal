// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SigilPoses_Overlay_PoseBased.generated.h"
struct FSigilAnimData_PoseOverlay_Layered;
/**
 * 
 */
UCLASS(BlueprintType, EditInlineNew, Const, CollapseCategories)
class SIGILMOVEMENT_API USigilPoses_Overlay_PoseBased : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="GMS", meta=(EditCondition="PoseOverlaySettingType==ESigilPoseOverlaySettingType::Layered", EditConditionHides))
	TArray<FSigilAnimData_PoseOverlay_Layered> LayeredPosesSetting;
	
};
