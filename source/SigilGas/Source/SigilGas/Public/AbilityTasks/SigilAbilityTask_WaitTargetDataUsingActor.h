// Copyright (c) 2026 Likeon. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "SigilAbilityTask_WaitTargetDataUsingActor.generated.h"


/**
 * 从一个已经生成的TargetActor中等待TargetData，当接收到有效数据后，并不销毁这个TargetActor。
 * 是原版本的WaitTargetData的重写，并添加了bCreateKeyIfNotValidForMorePredicting的功能。
 */
UCLASS()
class SIGILGAS_API USigilAbilityTask_WaitTargetDataUsingActor : public UAbilityTask
{
	GENERATED_BODY()
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitTargetDataUsingActorDelegate, const FGameplayAbilityTargetDataHandle&,Data);

	UPROPERTY(BlueprintAssignable)
	FWaitTargetDataUsingActorDelegate ValidData;

	UPROPERTY(BlueprintAssignable)
	FWaitTargetDataUsingActorDelegate Cancelled;

	/** 传入一个已经生成的TargetActor并等待返回有效数据或者取消,这个TargetActor在使用后不会被销毁。
	* 
	* @param bCreateKeyIfNotValidForMorePredicting Will create a new scoped prediction key if the current scoped prediction key is not valid for more predicting.
	* If false, it will always create a new scoped prediction key. We would want to set this to true if we want to use a potentially existing valid scoped prediction
	* key like the ability's activation key in a batched ability.
	*/
	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta=(HidePin = "OwningAbility", DefaultToSelf =
		"OwningAbility",
		BlueprintInternalUseOnly=
		"true", HideSpawnParms="Instigator"))
	static USigilAbilityTask_WaitTargetDataUsingActor* WaitTargetDataWithReusableActor(
		UGameplayAbility* OwningAbility, FName TaskInstanceName,
		TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType,
		AGameplayAbilityTargetActor* InTargetActor, bool bCreateKeyIfNotValidForMorePrediction = false);

	virtual void Activate() override;

	/** server处理TargetDataSet事件 */
	UFUNCTION()
	virtual void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data,
	                                            FGameplayTag ActivationTag);
	/** server处理TargetDataCancelled事件 */
	UFUNCTION()
	virtual void OnTargetDataReplicatedCancelledCallback();

	/** 玩家Confirm目标后触发(TargetActor->ConfirmTargeting) */
	UFUNCTION()
	virtual void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data);

	/** 玩家Cancel目标后触发(TargetActor->CancelTargeting) */
	UFUNCTION()
	virtual void OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data);

	// Called when the ability is asked to confirm from an outside node. What this means depends on the individual task. By default, this does nothing other than ending if bEndTask is true.
	virtual void ExternalConfirm(bool bEndTask) override;

	// Called when the ability is asked to cancel from an outside node. What this means depends on the individual task. By default, this does nothing other than ending the task.
	virtual void ExternalCancel() override;

protected:
	UPROPERTY()
	TObjectPtr<AGameplayAbilityTargetActor> TargetActor;

	bool bCreateKeyIfNotValidForMorePrediction;

	TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType;

	virtual void InitializeTargetActor() const;
	virtual void RegisterTargetDataCallbacks();
	virtual void FinalizeTargetActor() const;

	void OnDestroy(bool AbilityEnded) override;

	/**
	* 如果是客户端且传入的TargetActor不能在服务端产生TargetData则返回真
	*/
	virtual bool ShouldReplicateDataToServer() const;
};
