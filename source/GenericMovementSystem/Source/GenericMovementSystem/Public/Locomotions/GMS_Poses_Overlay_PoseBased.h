// Copyright RedMoon Games, Inc. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GMS_Poses_Overlay_PoseBased.generated.h"
struct FGMS_AnimData_PoseOverlay_Layered;
/**
 * 
 */
UCLASS(BlueprintType, EditInlineNew, Const, CollapseCategories)
class GENERICMOVEMENTSYSTEM_API UGMS_Poses_Overlay_PoseBased : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="GMS", meta=(EditCondition="PoseOverlaySettingType==EGMS_PoseOverlaySettingType::Layered", EditConditionHides))
	TArray<FGMS_AnimData_PoseOverlay_Layered> LayeredPosesSetting;
	
};
