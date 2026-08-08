// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilCameraMode.h"
#include "UObject/Object.h"
#include "SigilCameraModeStack.generated.h"


/**
 * USigilCameraModeStack
 *
 * Stack used for blending camera modes.
 */
UCLASS()
class SIGILCAMERA_API USigilCameraModeStack : public UObject
{
	GENERATED_BODY()

public:
	USigilCameraModeStack();

	void ActivateStack();
	void DeactivateStack();

	bool IsStackActivate() const { return bIsActive; }

	void PushCameraMode(TSubclassOf<USigilCameraMode> CameraModeClass);

	bool EvaluateStack(float DeltaTime, FSigilCameraModeView& OutCameraModeView);

	void DrawDebug(UCanvas* Canvas) const;

	// Gets the tag associated with the top layer and the blend weight of it
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:
	/**
	 * Get or create new camera mode. 
	 */
	USigilCameraMode* GetCameraModeInstance(TSubclassOf<USigilCameraMode> CameraModeClass);

	void UpdateStack(float DeltaTime);
	void BlendStack(FSigilCameraModeView& OutCameraModeView) const;

protected:
	bool bIsActive;

	/**
	 * Cached camera mode instance pool.
	 */
	UPROPERTY()
	TArray<TObjectPtr<USigilCameraMode>> CameraModeInstances;

	/**
	 * The list of active camera mode.
	 */
	UPROPERTY()
	TArray<TObjectPtr<USigilCameraMode>> CameraModeStack;
};
