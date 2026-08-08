// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SigilANS_MovementCancellation.generated.h"

/**
 * Animation notify state to disable montage root motion when moving.
 * 当角色移动时禁用蒙太奇根运动的动画通知状态。
 */
UCLASS(BlueprintType, Blueprintable, Abstract, HideDropdown)
class SIGILCOMBAT_API USigilANS_MovementCancellation : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	USigilANS_MovementCancellation(const FObjectInitializer& ObjectInitializer);

	/**
	 * Called when the notify begins.
	 * 通知开始时调用。
	 * @param BranchingPointPayload The notify payload. 通知载荷。
	 */
	virtual void BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload) override;

	/**
	 * Called each frame during the notify.
	 * 通知期间每帧调用。
	 * @param BranchingPointPayload The notify payload. 通知载荷。
	 * @param FrameDeltaTime Time since last frame. 上一帧以来的时间。
	 */
	virtual void BranchingPointNotifyTick(FBranchingPointNotifyPayload& BranchingPointPayload, float FrameDeltaTime) override;

	/**
	 * Called when the notify ends.
	 * 通知结束时调用。
	 * @param BranchingPointPayload The notify payload. 通知载荷。
	 */
	virtual void BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload) override;

protected:
	/**
	 * Checks if the skeletal mesh is moving.
	 * 检查骨骼网格是否在移动。
	 * @param MeshComp The skeletal mesh component. 骨骼网格组件。
	 * @return True if moving. 如果在移动返回true。
	 */
	UFUNCTION(BlueprintNativeEvent)
	bool IsMoving(USkeletalMeshComponent* MeshComp) const;
	virtual bool IsMoving_Implementation(USkeletalMeshComponent* MeshComp) const;

	/**
	 * Indicates if root motion is disabled.
	 * 表示根运动是否被禁用。
	 */
	bool IsRootMotionDisabled{false};

#if WITH_EDITOR
	/**
	 * Checks if the notify can be placed on an animation.
	 * 检查通知是否可以放置在动画上。
	 * @param Animation The animation sequence. 动画序列。
	 * @return True if valid. 如果有效返回true。
	 */
	virtual bool CanBePlaced(UAnimSequenceBase* Animation) const override;
#endif
};
