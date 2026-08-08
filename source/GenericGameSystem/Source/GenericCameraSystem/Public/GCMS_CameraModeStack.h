// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GCMS_CameraMode.h"
#include "UObject/Object.h"
#include "GCMS_CameraModeStack.generated.h"


/**
 * UGCMS_CameraModeStack
 *
 * Stack used for blending camera modes.
 */
UCLASS()
class GENERICCAMERASYSTEM_API UGCMS_CameraModeStack : public UObject
{
	GENERATED_BODY()

public:
	UGCMS_CameraModeStack();

	void ActivateStack();
	void DeactivateStack();

	bool IsStackActivate() const { return bIsActive; }

	void PushCameraMode(TSubclassOf<UGCMS_CameraMode> CameraModeClass);

	void PopCameraMode(TSubclassOf<UGCMS_CameraMode> CameraModeClass);

	bool EvaluateStack(float DeltaTime, FGCMS_CameraModeView& OutCameraModeView);

	void DrawDebug(UCanvas* Canvas) const;

	// Gets the tag associated with the top layer and the blend weight of it
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:
	/**
	 * Get or create new camera mode. 
	 */
	UGCMS_CameraMode* GetCameraModeInstance(TSubclassOf<UGCMS_CameraMode> CameraModeClass);

	void UpdateStack(float DeltaTime);
	void BlendStack(FGCMS_CameraModeView& OutCameraModeView) const;

protected:
	bool bIsActive;

	/**
	 * Cached camera mode instance pool.
	 */
	UPROPERTY()
	TArray<TObjectPtr<UGCMS_CameraMode>> CameraModeInstances;

	/**
	 * The list of active camera mode.
	 */
	UPROPERTY()
	TArray<TObjectPtr<UGCMS_CameraMode>> CameraModeStack;
};
