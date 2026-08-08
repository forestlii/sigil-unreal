// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "Engine/World.h"
#include "GameplayTagContainer.h"

#include "GCMS_CameraMode.generated.h"

class USpringArmComponent;
class UCameraComponent;
class AActor;
class UCanvas;

/**
 * EGCMS_CameraModeBlendFunction
 *
 *	Blend function used for transitioning between camera modes.
 */
UENUM(BlueprintType)
enum class EGCMS_CameraModeBlendFunction : uint8
{
	// Does a simple linear interpolation.
	Linear,

	// Immediately accelerates, but smoothly decelerates into the target.  Ease amount controlled by the exponent.
	EaseIn,

	// Smoothly accelerates, but does not decelerate into the target.  Ease amount controlled by the exponent.
	EaseOut,

	// Smoothly accelerates and decelerates.  Ease amount controlled by the exponent.
	EaseInOut,

	COUNT UMETA(Hidden)
};


/**
 * GCMS_CameraModeView
 *
 * View data produced by the camera mode that is used to blend camera modes.
 */
USTRUCT(BlueprintType)
struct GENERICCAMERASYSTEM_API FGCMS_CameraModeView
{
public:
	GENERATED_BODY()

	FGCMS_CameraModeView();

	void Blend(const FGCMS_CameraModeView& Other, float OtherWeight);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCMS")
	FVector Location{ForceInit};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCMS")
	FRotator Rotation{ForceInit};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCMS")
	FVector SprintArmSocketOffset{ForceInit};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCMS")
	FVector SprintArmTargetOffset{ForceInit};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCMS")
	float SprintArmLength{ForceInit};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCMS")
	FRotator ControlRotation{ForceInit};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCMS")
	float FieldOfView{ForceInit};
};


/**
 * UGCMS_CameraMode
 *
 *	Base class for all camera modes.
 */
UCLASS(Abstract, Blueprintable)
class GENERICCAMERASYSTEM_API UGCMS_CameraMode : public UObject
{
	GENERATED_BODY()

public:
	UGCMS_CameraMode();

	virtual UWorld* GetWorld() const override;

	/**
	 * @return Returns the target actor that owning camera is looking at. 返回所属相机当前跟随的目标Actor。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="CameraMode")
	AActor* GetTargetActor() const;

	const FGCMS_CameraModeView& GetCameraModeView() const { return View; }

	// Called when this camera mode is activated on the camera mode stack.
	virtual void OnActivation()
	{
		K2_OnActivation();
	};

	// Called when this camera mode is deactivated on the camera mode stack.
	virtual void OnDeactivation()
	{
		K2_OnDeactivation();
	};

	void UpdateCameraMode(float DeltaTime);

	float GetBlendTime() const { return BlendTime; }
	float GetBlendWeight() const { return BlendWeight; }
	void SetBlendWeight(float Weight);

	FGameplayTag GetCameraTypeTag() const
	{
		return CameraTypeTag;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="CameraMode")
	UCameraComponent* GetAssociatedCamera() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="CameraMode")
	USpringArmComponent* GetAssociatedSprintArm() const;

	virtual void DrawDebug(UCanvas* Canvas) const;

protected:
	/**
	 * Called when this camera mode activated.
	 * 在此相机模式激活时调用。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="CameraMode", meta=(DisplayName="OnActivation"))
	void K2_OnActivation();

	/**
	 * Called when this camera mode deactivated.
	 * 在此相机模式禁用时调用。 
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="CameraMode", meta=(DisplayName="OnDeactivation"))
	void K2_OnDeactivation();

	/**
	 * Return the pivot location of this camera mode.
	 * @details Default will return Character's capsule location(Taking crouching state in count), or fallback to Pawn's ViewLocation or fallback to PawnLocation.
	 * 返回此相机模式的轴心位置。
	 * @细节，默认实现是返回考虑到Character的蹲伏状态的胶囊体中心点，或者回退到Pawn的ViewLocation，或者回退到Pawn的Location。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="CameraMode")
	FVector GetPivotLocation() const;

	/**
	 * Return the pivot rotation of this camera mode.
	 * @details Default will return TargetActor(Pawn)'s view rotation or fallback to actor rotation.
	 * 返回此相机模式的轴心旋转。
	 * @细节 默认会返回TargetActor(Pawn)的ViewRotation,或者回退到Actor的旋转。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="CameraMode")
	FRotator GetPivotRotation() const;

	virtual void UpdateView(float DeltaTime);

	virtual void UpdateBlending(float DeltaTime);

	/**
	 * This is where you update View(CameraModeView)
	 * @param DeltaTime 
	 * @param PivotLocation Location returned from GetPivotLocation
	 * @param PivotRotation Rotation returned from GetPivotRotation
	 */
	UFUNCTION(BlueprintNativeEvent, Category="CameraMode")
	void OnUpdateView(float DeltaTime, FVector PivotLocation, FRotator PivotRotation);
	virtual void OnUpdateView_Implementation(float DeltaTime, FVector PivotLocation, FRotator PivotRotation);

protected:
	// A tag that can be queried by gameplay code that cares when a kind of camera mode is active
	// without having to ask about a specific mode (e.g., when aiming downsights to get more accuracy)
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	FGameplayTag CameraTypeTag;

	// View output produced by the camera mode.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="View")
	FGCMS_CameraModeView View;

	// The horizontal field of view (in degrees).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "5.0", ClampMax = "170.0"))
	float FieldOfView;

	// Minimum view pitch (in degrees).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMin;

	// Maximum view pitch (in degrees).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMax;

	/**
	 * How long it takes to blend in this mode.
	 * 此相机模式的混入时间。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendTime;

	/**
	 * Function used for blending.
	 * 用于混合的方式。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	EGCMS_CameraModeBlendFunction BlendFunction;

	// Exponent used by blend functions to control the shape of the curve.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendExponent;

	// Linear blend alpha used to determine the blend weight.
	UPROPERTY(VisibleInstanceOnly, Category="Blending")
	float BlendAlpha;

	// Blend weight calculated using the blend alpha and function.
	UPROPERTY(VisibleInstanceOnly, Category="Blending")
	float BlendWeight;

	UPROPERTY(EditDefaultsOnly, Category = "Duration")
	float ActiveTime;

	/**
	 * The max active time of this camera mode, When active time reach this value, this camera mode will auto popup.
	 * 此相机模式的最大激活时间，当当前激活时间到达此时长，会返回到默认相机模式。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Duraction")
	float MaxActiveTime;

protected:
	/** If true, skips all interpolation and puts camera in ideal location.  Automatically set to false next frame. */
	UPROPERTY(transient)
	uint32 bResetInterpolation : 1;
};
