// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/CachedAnimData.h"
#include "UObject/Object.h"
#include "SigilLocomotionStructLibrary.generated.h"

class UBlendSpace1D;
class UAnimSequenceBase;
class UAnimSequence;


USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilTaggedStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="GMS")
	FGameplayTag Tag;

	bool operator==(const FGameplayTag& Other) const
	{
		return Tag == Other;
	}

	bool operator!=(const FGameplayTag& Other) const
	{
		return Tag != Other;
	}
};


USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilLocomotionState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bHasInput{false};

	/**
	 * Input yaw angle in world space.
	 * 输入的偏航角度（世界空间）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float InputYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bHasSpeed{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float Speed{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector Velocity{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector PreviousVelocity{ForceInit};

	/**
	 * Yaw angle of character velocity.
	 * 速率本身的偏航角度
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float VelocityYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector CurrentAcceleration{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bMoving{false};

	/**
	 * The target yaw angle that the Actor should rotate to.
	 * Actor应该旋转至的目标偏航角。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float TargetYawAngle{0.0f};

	// Used for extra smooth actor rotation, in other cases equal to the regular target yaw angle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float SmoothTargetYawAngle{0.0f};

	/**
	 * Angle between View Yaw Angle Relative to Target Yaw Angle
	 * 视角偏航角与目标偏航角之间的角度
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float ViewRelativeTargetYawAngle{0.0f};

	/**
	 * Location of actor in WordSpace.
	 * Actor在世界空间的位置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector Location{ForceInit};

	/**
	 * Rotation of actor in WordSpace.
	 * Actor在世界空间的旋转。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FRotator Rotation{ForceInit};

	/**
	 * The Rotation quat of actor in WordSpace.
	 * Actor在世界空间的旋转四元数。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FQuat RotationQuaternion{ForceInit};

	/**
	 * The yaw angle of the Actor at the last frame.
	 * Actor在上一帧时的偏航角。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float PreviousYawAngle{0.0f};

	/**
	 * The change speed of actor's rotation yaw.
	 * Actor的偏航角变化速度。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ForceUnits = "deg/s"))
	float YawSpeed{0.0f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float AimingYawAngleLimit{180.0f};
};


USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilViewState
{
	GENERATED_BODY()

	/**
	 * The smoothed ViewRotation(set by ReplicatedViewRotation)
	 * 视角的旋转。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FRotator Rotation{ForceInit};

	/**
	 * This represents the speed the camera is rotating from left to right.
	 * 表示视角左右移动的速度。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float YawSpeed{0.0f};

	/**
	 * The angle between the view rotation yaw and the actor rotation yaw.
	 * 视角偏航角与Actor偏航角之间的角度。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float YawAngle{0.0f};

	/**
	 * The View rotation yaw in last frame.
	 * 上一帧的视角偏航角角度。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float PreviousYawAngle{0.0f};
};


USTRUCT()
struct FSigilPredictGroundMovementStopLocationParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="GMS")
	FVector Velocity{ForceInit};
	UPROPERTY(EditAnywhere, Category="GMS")
	bool bUseSeparateBrakingFriction{ForceInit};
	UPROPERTY(EditAnywhere, Category="GMS")
	float BrakingFriction{ForceInit};
	UPROPERTY(EditAnywhere, Category="GMS")
	float GroundFriction{ForceInit};
	UPROPERTY(EditAnywhere, Category="GMS")
	float BrakingFrictionFactor{ForceInit};
	UPROPERTY(EditAnywhere, Category="GMS")
	float BrakingDecelerationWalking{ForceInit};
};

USTRUCT()
struct FSigilPredictGroundMovementPivotLocationParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="GMS")
	FVector Acceleration{ForceInit};
	UPROPERTY(EditAnywhere, Category="GMS")
	FVector Velocity{ForceInit};
	UPROPERTY(EditAnywhere, Category="GMS")
	float GroundFriction{0.0f};
};


USTRUCT(BlueprintType)
struct FSigilAnimations_4Direction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Forward = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Backward = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Left = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Right = nullptr;

	bool ValidAnimations() const;
	bool HasRootMotion() const;
};

USTRUCT(BlueprintType)
struct FSigilAnimations_8Direction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Forward = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> ForwardLeft = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> ForwardRight = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Backward = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> BackwardLeft = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> BackwardRight = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Left = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Right = nullptr;

	bool ValidAnimations() const;
	bool HasRootMotion() const;
};

USTRUCT(BlueprintType)
struct FSigilAnimations_BS1D_FwdBwd
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UBlendSpace1D> Forward{nullptr};

	UPROPERTY(EditAnywhere, Category="GMS")
	TObjectPtr<UBlendSpace1D> Backward{nullptr};
};


USTRUCT(BlueprintType)
struct FSigilAnimations_StartForwardFacing
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> StartForward = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> StartForwardL90 = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> StartForwardR90 = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> StartForwardL180 = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> StartForwardR180 = nullptr;
};

USTRUCT(BlueprintType)
struct FSigilAnimations_StartForwardFacing_8Direction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> StartForward = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> StartForwardL90 = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> StartForwardR90 = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> StartForwardL180 = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> StartForwardR180 = nullptr;
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimationWithDistance
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS")
	TObjectPtr<UAnimSequence> Animation = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS", meta=(ClampMin=0))
	float Distance{200};

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category=AlwaysHidden, Meta=(EditCondition=False, EditConditionHides))
	FString EditorFriendlyName;
#endif
};

USTRUCT(BlueprintType)
struct FSigilAnimStateNameToTag
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(Categories="Sigil.Movement.SM"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "GMS")
	FCachedAnimStateData State;

	friend bool operator==(const FSigilAnimStateNameToTag& Lhs, const FSigilAnimStateNameToTag& RHS)
	{
		return Lhs.Tag == RHS.Tag
			&& Lhs.State.StateMachineName == RHS.State.StateMachineName && Lhs.State.StateName == RHS.State.StateName;
	}

	friend bool operator!=(const FSigilAnimStateNameToTag& Lhs, const FSigilAnimStateNameToTag& RHS)
	{
		return !(Lhs == RHS);
	}
};

USTRUCT()
struct FSigilAnimStateNameToTagWrapper
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FSigilAnimStateNameToTag> AnimStateNameToTagMapping;
};
