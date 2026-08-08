// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilAnimLayer.h"
#include "SigilAnimLayer_View.h"
#include "SigilAnimLayer_View_Default.generated.h"

class UAimOffsetBlendSpace;

/**
 * Native default view anim layer setting implementation.
 */
UCLASS(NotBlueprintable)
class USigilAnimLayerSetting_View_Default final : public USigilAnimLayerSetting_View
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
class SIGILMOVEMENT_API USigilAnimLayer_View_Default : public USigilAnimLayer
{
	GENERATED_BODY()

public:
	virtual void ApplySetting_Implementation(const USigilAnimLayerSetting* Setting) override;
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
