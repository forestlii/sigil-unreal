// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Components/SkeletalMeshComponent.h"
#include "SigilAbilityTask_CollisionTrace.generated.h"

class USigilCollisionTraceInstance;
class USigilAttackRequest_Melee;

/**
 * Ability task for handling collision traces in combat.
 * 处理战斗中碰撞检测的能力任务。
 */
UCLASS()
class SIGILCOMBAT_API USigilAbilityTask_CollisionTrace : public UAbilityTask
{
	GENERATED_BODY()

public:
	/**
	 * Creates and activates a collision trace task.
	 * 创建并激活碰撞检测任务。
	 * @param OwningAbility The owning gameplay ability. 所属游戏能力。
	 * @param TaskInstanceName The name of the task instance. 任务实例名称。
	 * @param bAdjustVisibilityBasedAnimTickOption Whether to adjust visibility-based animation ticking. 是否调整基于可见性的动画tick。
	 * @return The created task. 创建的任务。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|AbilityTasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static USigilAbilityTask_CollisionTrace* HandleCollisionTraces(UGameplayAbility* OwningAbility, FName TaskInstanceName, bool bAdjustVisibilityBasedAnimTickOption = false);

	/**
	 * Activates the task.
	 * 激活任务。
	 */
	virtual void Activate() override;

	/**
	 * Called when the task is destroyed.
	 * 任务销毁时调用。
	 * @param bInOwnerFinished Whether the owner finished the task. 拥有者是否完成了任务。
	 */
	virtual void OnDestroy(bool bInOwnerFinished) override;

	/**
	 * Adds a melee attack request to the task.
	 * 向任务添加近战攻击请求。
	 * @param Request The melee attack request. 近战攻击请求。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|AbilityTasks")
	void AddMeleeRequest(const USigilAttackRequest_Melee* Request);

	/**
	 * Removes a melee attack request from the task.
	 * 从任务移除近战攻击请求。
	 * @param Request The melee attack request. 近战攻击请求。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|AbilityTasks")
	void RemoveMeleeRequest(const USigilAttackRequest_Melee* Request);

	/**
	 * Delegate for trace instance hit events.
	 * 碰撞检测实例命中事件的委托。
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSigilOnTraceInstanceHitSignature, const USigilAttackRequest_Melee*, MeleeRequest, USigilCollisionTraceInstance*, TraceInstance, const FHitResult&, HitResult);

	/**
	 * Fired when a trace instance detects targets.
	 * 当碰撞检测实例检测到目标时触发。
	 */
	UPROPERTY(BlueprintAssignable)
	FSigilOnTraceInstanceHitSignature OnTargetsFound;

protected:
	/**
	 * Handles trace instance hit events.
	 * 处理碰撞检测实例命中事件。
	 * @param TraceInstance The trace instance. 碰撞检测实例。
	 * @param HitResult The hit result. 命中结果。
	 */
	UFUNCTION()
	void TraceInstanceHitCallback(USigilCollisionTraceInstance* TraceInstance, const FHitResult& HitResult);

	/**
	 * Map of melee requests to their associated trace instances.
	 * 近战请求及其关联碰撞检测实例的映射。
	 */
	TMap<TObjectPtr<const USigilAttackRequest_Melee>, TArray<TObjectPtr<USigilCollisionTraceInstance>>> MeleeRequests;

	/**
	 * Whether to adjust visibility-based animation ticking.
	 * 是否调整基于可见性的动画tick。
	 */
	UPROPERTY()
	bool bAdjustAnimTickOption{false};

	/**
	 * Whether the animation tick option was adjusted.
	 * 是否已调整动画tick选项。
	 */
	UPROPERTY()
	bool bAdjustedAnimTickOption{false};

	/**
	 * The previous animation tick option.
	 * 之前的动画tick选项。
	 */
	UPROPERTY()
	EVisibilityBasedAnimTickOption PrevAnimTickOption{EVisibilityBasedAnimTickOption::AlwaysTickPose};
};