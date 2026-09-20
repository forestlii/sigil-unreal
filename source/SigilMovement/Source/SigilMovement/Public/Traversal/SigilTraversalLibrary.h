// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Traversal/SigilTraversalTypes.h"
#include "SigilTraversalLibrary.generated.h"

class AActor;

/**
 * Finds out whether the obstacle in front of a character can be traversed, and how.
 * Everything except CheckTraversal is a pure function, so the rules can be unit tested without a world.
 * 判断角色面前的障碍物能否越过、用哪种动作。除 CheckTraversal 外都是纯函数，规则无需世界即可单测。
 */
UCLASS()
class SIGILMOVEMENT_API USigilTraversalLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Forward probe distance. Grounded: forward speed 0..500 maps to 75..350. Airborne: always 75.
	 * 前探距离。地面：朝向上的速度 0 到 500 映射为 75 到 350。空中：固定 75。
	 */
	UFUNCTION(BlueprintPure, Category = "Sigil|Traversal")
	static float ComputeForwardTraceDistance(float ForwardSpeed, bool bGrounded);

	/**
	 * Picks the action from measurements. Rows, first match wins (all need a front ledge):
	 * thin + back floor + drop over HurdleMin -> Hurdle; thin + back floor + drop under StepUpMax -> Mantle;
	 * thin + no back floor -> Vault; deeper than PlatformMinDepth -> Mantle.
	 * Height limits then apply: MaxGroundedHeight / MaxAirborneHeight overall, MaxHurdleAndVaultHeight for grounded hurdles and vaults.
	 * 由测量值定动作，自上而下取第一条满足的；随后套用高度上限。
	 */
	UFUNCTION(BlueprintPure, Category = "Sigil|Traversal")
	static ESigilTraversalAction ClassifyAction(
		const FSigilTraversalCheckResult& Measured,
		const FSigilTraversalRules& Rules,
		bool bGrounded);

	/**
	 * Ledges of an oriented box for someone approaching it: the top edge of the face nearest to the instigator,
	 * and the top edge of the opposite face. Works for any rotation and scale.
	 * 有朝向的盒子面向来者的边缘：离来者最近那个侧面的顶边，及其对面的顶边。适用于任意旋转与缩放。
	 *
	 * @param BoxTransform  Box centre, rotation and scale. 盒心、旋转与缩放。
	 * @param BoxExtent     Unscaled half size. 未缩放的半尺寸。
	 */
	UFUNCTION(BlueprintPure, Category = "Sigil|Traversal")
	static bool ComputeBoxLedges(
		const FTransform& BoxTransform,
		const FVector& BoxExtent,
		const FVector& HitLocation,
		const FVector& InstigatorLocation,
		FSigilTraversalLedges& OutLedges);

	/**
	 * The whole check: probe forward, ask the obstacle for its ledges, make sure the character fits on top,
	 * measure height, depth and the floor behind, then classify.
	 * 完整检测：前探、向障碍物要边缘、确认顶面放得下角色、量高度进深与后方地面、定动作。
	 *
	 * @param FeetLocation  The character's feet, used as the height reference. 角色脚底，作为高度基准。
	 * @return true when OutResult.Action is not None. OutResult.Action 不为 None 时返回真。
	 */
	UFUNCTION(BlueprintCallable, Category = "Sigil|Traversal")
	static bool CheckTraversal(
		AActor* Instigator,
		const FVector& FeetLocation,
		const FSigilTraversalCheckInputs& Inputs,
		const FSigilTraversalRules& Rules,
		bool bGrounded,
		FSigilTraversalCheckResult& OutResult);
};
