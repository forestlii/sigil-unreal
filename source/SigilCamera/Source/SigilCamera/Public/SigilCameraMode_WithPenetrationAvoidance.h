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
	/**
	 * Runs penetration avoidance for the current View and folds the result into the view's spring-arm parameters.
	 * Coordinate ownership: the camera's final transform is produced by the SpringArm that USigilCameraSystemComponent
	 * drives, so this function does NOT move the camera directly. It computes the desired camera location from
	 * View.Location (pivot) + SpringArm rotation/length/offsets, feels along the aim line, and shortens
	 * View.SpringArmLength proportionally to the blocked distance. Disable the SpringArm's own bDoCollisionTest when
	 * using this, otherwise both systems push the camera in.
	 * Call it from OnUpdateView after the view has been filled in.
	 * 对当前 View 执行穿模规避并把结果折算进弹簧臂参数。坐标归属：最终相机变换由 USigilCameraSystemComponent
	 * 驱动的 SpringArm 生成，本函数不直接移动相机——它按 View.Location（枢轴）+ 弹簧臂旋转/长度/偏移算出期望
	 * 相机位置，沿瞄准线做探测，再按被遮挡比例缩短 View.SpringArmLength。使用时请关闭 SpringArm 自带的
	 * bDoCollisionTest，否则两套系统会叠加推近。请在 OnUpdateView 填完 View 之后调用。
	 */
	UFUNCTION(BlueprintCallable, Category="CameraMode|PenetrationAvoidance")
	void UpdatePreventPenetration(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category="CameraMode|PenetrationAvoidance")
	void PreventCameraPenetration(bool bSingleRayOnly, const float& DeltaTime, const AActor* ViewTarget, const FVector& SafeLoc, FVector& CameraLoc, float& DistBlockedPct);

	virtual void DrawDebug(UCanvas* Canvas) const override;
};
