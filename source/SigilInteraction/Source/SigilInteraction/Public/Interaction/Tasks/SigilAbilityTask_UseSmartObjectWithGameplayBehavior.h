// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "SmartObjectRuntime.h"
#include "SmartObjectTypes.h"
#include "SigilAbilityTask_UseSmartObjectWithGameplayBehavior.generated.h"

class UGameplayBehavior;

/**
 * Ability task for using a smart object with gameplay behavior.
 * 使用智能对象和游戏行为的技能任务。
 */
UCLASS()
class SIGILINTERACTION_API USigilAbilityTask_UseSmartObjectWithGameplayBehavior : public UAbilityTask
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the ability task.
	 * 技能任务构造函数。
	 */
	USigilAbilityTask_UseSmartObjectWithGameplayBehavior(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Creates an ability task to use a smart object with gameplay behavior.
	 * 创建使用智能对象和游戏行为的技能任务。
	 * @param OwningAbility The owning gameplay ability. 拥有的游戏技能。
	 * @param ClaimHandle The smart object claim handle. 智能对象认领句柄。
	 * @param ClaimPriority The claim priority. 认领优先级。
	 * @return The created ability task. 创建的技能任务。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGS|Interaction", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static USigilAbilityTask_UseSmartObjectWithGameplayBehavior* UseSmartObjectWithGameplayBehavior(UGameplayAbility* OwningAbility, FSmartObjectClaimHandle ClaimHandle,
	                                                                                               ESmartObjectClaimPriority ClaimPriority = ESmartObjectClaimPriority::Normal);

	/**
	 * Sets the smart object claim handle.
	 * 设置智能对象认领句柄。
	 * @param Handle The claim handle. 认领句柄。
	 */
	void SetClaimHandle(const FSmartObjectClaimHandle& Handle) { ClaimedHandle = Handle; }

	/**
	 * Delegate for when the interaction succeeds.
	 * 交互成功时的委托。
	 */
	UPROPERTY(BlueprintAssignable)
	FGenericGameplayTaskDelegate OnSucceeded;

	/**
	 * Delegate for when the interaction fails.
	 * 交互失败时的委托。
	 */
	UPROPERTY(BlueprintAssignable)
	FGenericGameplayTaskDelegate OnFailed;

protected:
	/**
	 * Activates the ability task.
	 * 激活技能任务。
	 */
	virtual void Activate() override;

	/**
	 * Called when the task is destroyed.
	 * 任务销毁时调用。
	 * @param bInOwnerFinished Whether the owner finished the task. 拥有者是否完成任务。
	 */
	virtual void OnDestroy(bool bInOwnerFinished) override;

	/**
	 * Starts the interaction with the smart object.
	 * 开始与智能对象的交互。
	 * @return True if the interaction started successfully, false otherwise. 如果交互成功开始返回true，否则返回false。
	 */
	bool StartInteraction();

	/**
	 * Called when the smart object behavior finishes.
	 * 智能对象行为完成时调用。
	 * @param Behavior The gameplay behavior. 游戏行为。
	 * @param Avatar The avatar actor. 化身演员。
	 * @param bInterrupted Whether the behavior was interrupted. 行为是否被中断。
	 */
	void OnSmartObjectBehaviorFinished(UGameplayBehavior& Behavior, AActor& Avatar, const bool bInterrupted);

	/**
	 * Called when the smart object slot is invalidated.
	 * 智能对象槽失效时调用。
	 * @param ClaimHandle The claim handle. 认领句柄。
	 * @param State The slot state. 槽状态。
	 */
	void OnSlotInvalidated(const FSmartObjectClaimHandle& ClaimHandle, const ESmartObjectSlotState State);

	/**
	 * The gameplay behavior for the interaction.
	 * 交互的游戏行为。
	 */
	UPROPERTY()
	TObjectPtr<UGameplayBehavior> GameplayBehavior;

	/**
	 * The claimed smart object handle.
	 * 认领的智能对象句柄。
	 */
	FSmartObjectClaimHandle ClaimedHandle;

	/**
	 * Delegate handle for behavior finished notification.
	 * 行为完成通知的委托句柄。
	 */
	FDelegateHandle OnBehaviorFinishedNotifyHandle;

	/**
	 * Indicates if the behavior has finished.
	 * 表示行为是否已完成。
	 */
	bool bBehaviorFinished;
};
