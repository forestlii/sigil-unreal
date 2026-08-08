// Copyright 2024 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GMS_AnimLayer.h"
#include "GMS_AnimLayer_View.h"
#include "GMS_AnimLayer_View_Default.generated.h"

class UAimOffsetBlendSpace;

/**
 * Native default view anim layer setting implementation.
 */
UCLASS(NotBlueprintable)
class UGMS_AnimLayerSetting_View_Default final : public UGMS_AnimLayerSetting_View
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Anim View")
	TObjectPtr<UBlendSpace> BlendSpace;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Anim View")
	float YawAngleOffset = 0.0f;

	/**
	 * Extras smooth for AO. Zero means not additional smooth.
	 * AO的额外平滑速度，填0意味着没有平滑。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Anim View",meta=(ClampMin=0))
	float SmoothInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Anim View")
	FVector2D YawAngleLimit{-90.0f, 90.0f};
};

/**
 * Native default view anim layer implementation.
 */
UCLASS(Abstract)
class GENERICMOVEMENTSYSTEM_API UGMS_AnimLayer_View_Default : public UGMS_AnimLayer
{
	GENERATED_BODY()

public:
	virtual void ApplySetting_Implementation(const UGMS_AnimLayerSetting* Setting) override;
	virtual void ResetSetting_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Anim View")
	TObjectPtr<UBlendSpace> BlendSpace;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim View")
	float YawAngleOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim View")
	FVector2D YawAngleLimit{ -90.0f, 90.0f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Anim View",meta=(ClampMin=0))
	float SmoothInterpSpeed = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Anim View")
	bool bValidBlendSpace;
};
