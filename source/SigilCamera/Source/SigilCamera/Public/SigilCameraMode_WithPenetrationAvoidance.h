// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DrawDebugHelpers.h"
#include "SigilCameraMode.h"
#include "SigilCameraPenetrationAvoidanceFeeler.h"
#include "SigilCameraMode_WithPenetrationAvoidance.generated.h"


/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class SIGILCAMERA_API USigilCameraMode_WithPenetrationAvoidance : public USigilCameraMode
{
	GENERATED_BODY()

public:
	USigilCameraMode_WithPenetrationAvoidance();

	// Penetration prevention
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PenetrationAvoidance")
	float PenetrationBlendInTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PenetrationAvoidance")
	float PenetrationBlendOutTime = 0.15f;

	/** If true, does collision checks to keep the camera out of the world. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PenetrationAvoidance")
	bool bPreventPenetration = true;

	/** If true, try to detect nearby walls and move the camera in anticipation.  Helps prevent popping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PenetrationAvoidance")
	bool bDoPredictiveAvoidance = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PenetrationAvoidance")
	float CollisionPushOutDistance = 2.f;

	/** When the camera's distance is pushed into this percentage of its full distance due to penetration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PenetrationAvoidance")
	float ReportPenetrationPercent = 0.f;

	/**
	 * These are the feeler rays that are used to find where to place the camera.
	 * Index: 0  : This is the normal feeler we use to prevent collisions.
	 * Index: 1+ : These feelers are used if you bDoPredictiveAvoidance=true, to scan for potential impacts if the player
	 *             were to rotate towards that direction and primitively collide the camera so that it pulls in before
	 *             impacting the occluder.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "PenetrationAvoidance")
	TArray<FSigilCameraPenetrationAvoidanceFeeler> PenetrationAvoidanceFeelers;

	UPROPERTY(Transient)
	float AimLineToDesiredPosBlockedPct;

	UPROPERTY(Transient)
	TArray<TObjectPtr<const AActor>> DebugActorsHitDuringCameraPenetration;

#if ENABLE_DRAW_DEBUG
	mutable float LastDrawDebugTime = -MAX_FLT;
#endif

protected:
	UFUNCTION(BlueprintCallable, Category="CameraMode|PenetrationAvoidance")
	void UpdatePreventPenetration(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category="CameraMode|PenetrationAvoidance")
	void PreventCameraPenetration(bool bSingleRayOnly, const float& DeltaTime, const AActor* ViewTarget, const FVector& SafeLoc, FVector& CameraLoc, float& DistBlockedPct);

	virtual void DrawDebug(UCanvas* Canvas) const override;
};
