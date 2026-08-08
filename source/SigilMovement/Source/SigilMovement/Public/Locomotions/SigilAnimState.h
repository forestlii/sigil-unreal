// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilLocomotionEnumLibrary.h"
#include "BoneControllers/AnimNode_OffsetRootBone.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/Object.h"
#include "SigilAnimState.generated.h"

class UAnimSequenceBase;
class UAnimSequence;

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimState_Locomotion
{
	GENERATED_BODY()

	// world location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector Location{ForceInit};

	UPROPERTY()
	float PreviousDisplacement{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float DisplacementSpeed{0.0f};

	// world rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FRotator Rotation{ForceInit};

	UPROPERTY()
	FQuat RotationQuaternion{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector Velocity{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector LocalVelocity2D{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bHasVelocity = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float Speed{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float TargetYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float LocalVelocityYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float LocalVelocityYawAngleWithOffset{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	ESigilMovementDirection LocalVelocityDirection{ESigilMovementDirection::Forward};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	ESigilMovementDirection LocalVelocityDirectionNoOffset{ESigilMovementDirection::Forward};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	ESigilMovementDirection_8Way LocalVelocityOctagonalDirection{ESigilMovementDirection_8Way::Forward};

	//bHasAcceleration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bHasInput{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector VelocityAcceleration{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector LocalAcceleration2D{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bMoving{false};

	float Scale{1.0f};

	float CapsuleRadius{0.0f};

	float CapsuleHalfHeight{0.0f};

	float MaxAcceleration{0.0f};

	float MaxBrakingDeceleration{0.0f};

	float WalkableFloorZ{0.0f};
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimState_Root
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	EOffsetRootBoneMode RotationMode{EOffsetRootBoneMode::Release};

	/**
	 * The current world space transform from the offset root bone animgraph node
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FTransform Transform{FTransform::Identity};

	/**
	 * The current world space transform of root bone.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FTransform RootTransform{FTransform::Identity};

	/**
	 * Root rotation yaw angle relative to Actor rotation yaw angle, <0 left >0 right.
	 * 根旋转偏航角相对于Actor旋转偏航角的角度，<0左边 >0右边。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State")
	float YawOffset{0};

	/**
	 * How much the offset can deviate from the mesh component's rotation in degrees,Values lower than 0 disable this limit
	 */ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float MaxRotationError{-1.0f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float DeltaToActorRotation{0};

	UPROPERTY()
	FFloatSpringState SprintState;
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimState_TurnInPlace
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 bUpdatedThisFrame : 1 {false};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequenceBase> Animation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	uint8 bShouldTurn : 1 {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float AccumulatedTime{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ForceUnits = "x"))
	float PlayRate{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ForceUnits = "x"))
	float ScaledPlayRate{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ForceUnits = "s"))
	float ActivationDelay{0.0f};
	
	// The angle triggered turn in place.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float TriggeredAngle{0.0f};

	bool b180{false};
	
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimState_InAir
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ForceUnits = "cm/s"))
	float VerticalSpeed{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bJumping{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bFalling{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ForceUnits = "x"))
	float JumpPlayRate{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bValidGround{false};

	/**
	 * @brief Remaining distance to hit ground(还有多远到达地面)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(ForceUnits = "cm"))
	float GroundDistance{0.0f};

	/**
	 * @brief Remaining time to reach jump apex(还有多久到达跳跃巅峰)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1, ForceUnits = "s"))
	float TimeToJumpApex{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1, ForceUnits = "s"))
	float FallingTime{0.0f};
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimState_Idle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Animation{nullptr};

	// Is current animation looped?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bLoop{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float PlayRate{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float BlendTime{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UBlendProfile> BlendProfile{nullptr};
};


/**
 * @brief Runtime state of idle break.
 */
USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimState_IdleBreak
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float TimeUntilNextIdleBreak{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	int32 CurrentIdleBreakIndex{0};

	/**
	 * Interval of idle breaks.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float IdleBreakDelayTime{0.0f};
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimState_Lean
{
	GENERATED_BODY()

	/**
	 * horizontal acceleration amount.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -1, ClampMax = 1))
	float RightAmount{0.0f};

	/**
	 * vertical acceleration amount.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -1, ClampMax = 1))
	float ForwardAmount{0.0f};
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimState_Pivot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Animation;

	/**
	 * Pivot开始时的加速度
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector StartingAcceleration{ForceInit};


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float AccumulatedTime{ForceInit};
	
	/**
	 * Time at distance matching stop
	 * 距离匹配时Pivot的停止时间
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float TimeAtPivotStop{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float StrideWarpingAlpha{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector2D PlayRateClamp{0.f, 0.f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FVector Direction2D{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	ESigilMovementDirection DesiredDirection{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	ESigilMovementDirection InitialDirection{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	float RemainingCooldown{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	bool bMovingPerpendicularToInitialDirection{ForceInit};
};


USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimState_Start
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Animation{nullptr};

	/**
	 * The yaw delta of acceleration direction and root direction when start state entry.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float YawDeltaToAcceleration{ForceInit};

	/**
	 * The starting movement direction.
	 * 触发Start的初始（想去的）方向
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	ESigilMovementDirection LocalVelocityDirection{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float StrideWarpingAlpha{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector2D PlayRateClamp{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float OrientationAlpha{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float AccumulatedTime{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UBlendProfile> BlendProfile{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float TimeRemaining{0.0f};
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimState_Cycle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Animation{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float OrientationAlpha{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float StrideWarpingAlpha{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float PlayRate{1.0f};

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	// TObjectPtr<UBlendProfile> BlendProfile{nullptr};
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimState_Stop
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Animation{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float OrientationAlpha{1.0f};
};

/**
 * @brief Anim runtime information related to View(Camera/AI's control rotation)
 */
USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimState_View
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FRotator Rotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float YawAngle{0.0f};

	/**
	 * How fast YawAngle changes.
	 * 偏航角以多快的速度变化.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float YawSpeed{0.0f};

	/**
	 * 俯仰角度
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -90, ClampMax = 90, ForceUnits = "deg"))
	float PitchAngle{0.0f};

	/**
	 * 俯仰程度
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float PitchAmount{0.5f};
};


USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimState_Layering
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float HeadBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float HeadAdditiveBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float HeadSlotBlendAmount{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftAdditiveBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftSlotBlendAmount{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftLocalSpaceBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftMeshSpaceBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightAdditiveBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightSlotBlendAmount{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightLocalSpaceBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightMeshSpaceBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float HandLeftBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float HandRightBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float SpineBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float SpineAdditiveBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float SpineSlotBlendAmount{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float PelvisBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float PelvisSlotBlendAmount{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float LegsBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ClampMax = 1))
	float LegsSlotBlendAmount{1.0f};
};
