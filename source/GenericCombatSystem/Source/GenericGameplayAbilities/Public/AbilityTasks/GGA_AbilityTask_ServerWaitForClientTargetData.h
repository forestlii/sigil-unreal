// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GGA_AbilityTask_ServerWaitForClientTargetData.generated.h"


UCLASS()
class GENERICGAMEPLAYABILITIES_API UGGA_AbilityTask_ServerWaitForClientTargetData : public UAbilityTask
{
	
	GENERATED_UCLASS_BODY()
	UPROPERTY(BlueprintAssignable)
	FWaitTargetDataDelegate ValidData;

	/**
	 * The server side waits for target data from the client.
	 * Client execution of this node will return directly from ActivatePin and continue execution
	 * 服务端等待客户端的目标数据。
	 * 客户端执行这个节点会直接从ActivatePin返回并继续执行
	 */
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "GGA|Tasks")
	static UGGA_AbilityTask_ServerWaitForClientTargetData* ServerWaitForClientTargetData(UGameplayAbility* OwningAbility, FName TaskInstanceName, bool TriggerOnce);

	virtual void Activate() override;

	UFUNCTION()
	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag);

protected:
	virtual void OnDestroy(bool AbilityEnded) override;

	bool bTriggerOnce;
};
