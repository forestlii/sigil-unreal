// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilTraversalTypes.generated.h"

class UPrimitiveComponent;

/**
 * What kind of move gets a character past an obstacle.
 * 角色越过障碍物的动作类型。
 */
UENUM(BlueprintType)
enum class ESigilTraversalAction : uint8
{
	None,
	/** Thin obstacle, the floor behind is clearly lower: step over and drop. 薄障碍，后方地面明显更低：跨过去落下。 */
	Hurdle,
	/** Thin obstacle with no floor to land on behind it (a window, a gap): go through. 薄障碍，后方没有落脚地面（窗、缺口）：翻过去。 */
	Vault,
	/** A platform, or a thin obstacle whose far side is level with its top: climb up and stand. 平台，或后方与顶面齐平的薄障碍：爬上去站着。 */
	Mantle
};

/**
 * Where the forward probe goes. All vectors are world space.
 * 前探检测的参数，向量均为世界空间。
 */
USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilTraversalCheckInputs
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	FVector TraceForwardDirection = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float TraceForwardDistance = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	FVector TraceOriginOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	FVector TraceEndOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float TraceRadius = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float TraceHalfHeight = 60.0f;

	/**
	 * The character's own capsule, used to check that it fits on and behind the obstacle. The forward probe is usually
	 * smaller than this so that it skims over steps and the floor. Zero or less: use the probe's size.
	 * 角色自身的胶囊，用于检查障碍物顶面与后方放不放得下角色。前探用的胶囊通常比它小，好掠过台阶与地面。不大于零则沿用前探尺寸。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float CapsuleRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float CapsuleHalfHeight = 0.0f;

	/** Half height to retry the room check with when standing does not fit. Zero or less: half of the standing one. 站立放不下时改用的蹲姿半高；不大于零则取站立半高的一半。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float CrouchedHalfHeight = 0.0f;
};

/**
 * The edges an obstacle offers to someone coming at it: the near top edge and, if it has one, the far top edge.
 * Normals point away from the obstacle, horizontally.
 * 障碍物给来者的边缘：近侧顶边，以及（若有）远侧顶边。法线水平、指向障碍物外侧。
 */
USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilTraversalLedges
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	bool bHasFrontLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	FVector FrontLedgeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	FVector FrontLedgeNormal = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	bool bHasBackLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	FVector BackLedgeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	FVector BackLedgeNormal = FVector::ZeroVector;
};

/**
 * Everything measured about one obstacle, plus the action that fits it.
 * 对一个障碍物量到的全部数据，以及与之匹配的动作。
 */
USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilTraversalCheckResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Traversal")
	ESigilTraversalAction Action = ESigilTraversalAction::None;

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Traversal")
	FSigilTraversalLedges Ledges;

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Traversal")
	bool bHasBackFloor = false;

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Traversal")
	FVector BackFloorLocation = FVector::ZeroVector;

	/** Front ledge height above the character's feet. 前边缘相对角色脚底的高度。 */
	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Traversal")
	float ObstacleHeight = 0.0f;

	/** Horizontal distance between the front and back ledges. 前后边缘的水平距离。 */
	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Traversal")
	float ObstacleDepth = 0.0f;

	/** How far the floor behind sits below the back ledge. 后方地面低于后边缘多少。 */
	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Traversal")
	float BackLedgeHeight = 0.0f;

	/** The character only fits on top of the obstacle when crouched. 角色只有蹲下才放得进障碍物顶面的空间。 */
	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Traversal")
	bool bShouldCrouch = false;

	/** Capsule half height that passed the room checks: standing, or crouched when bShouldCrouch. 通过空间检查的胶囊半高：站立，或 bShouldCrouch 时的蹲姿。 */
	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Traversal")
	float FitHalfHeight = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Traversal")
	TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;
};

/**
 * Thresholds that turn measurements into an action. Defaults come from the four-row rule table of the reference design:
 * depth under 59 is "thin", a back ledge drop over 50 is a hurdle, under 10 is a step up, depth over 29 can be a platform.
 * 把测量值变成动作的阈值。默认值取自参考设计的四行判定表。
 */
USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilTraversalRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float ThinObstacleMaxDepth = 59.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float HurdleMinBackLedgeHeight = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float StepUpMaxBackLedgeHeight = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float PlatformMinDepth = 29.0f;

	/** Highest obstacle reachable from the ground. 地面状态能攀的最高障碍。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float MaxGroundedHeight = 275.0f;

	/** Highest ledge catchable while airborne. 空中状态能抓的最高边缘。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float MaxAirborneHeight = 200.0f;

	/** Thin obstacles above this can only be mantled from the ground. 超过此高度的薄障碍在地面状态只能攀上。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float MaxHurdleAndVaultHeight = 125.0f;

	/**
	 * A floor behind the obstacle only counts when it is no lower than this far below the character's feet;
	 * anything deeper is treated as "no floor" (a window with a drop outside is vaulted, not hurdled).
	 * 障碍物后方的地面，低于角色脚底不超过这个值才算「有落脚地面」；更深的视为没有（外面有落差的窗走翻越而不是跨栏）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	float BackFloorMaxDropBelowFeet = 50.0f;
};
