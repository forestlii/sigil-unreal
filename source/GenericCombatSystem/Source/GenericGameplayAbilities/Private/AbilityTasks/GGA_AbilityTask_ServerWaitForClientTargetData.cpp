// Copyright 2025 RedMoonGames All Rights Reserved.


#include "AbilityTasks/GGA_AbilityTask_ServerWaitForClientTargetData.h"
#include "AbilitySystemComponent.h"

UGGA_AbilityTask_ServerWaitForClientTargetData::UGGA_AbilityTask_ServerWaitForClientTargetData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGGA_AbilityTask_ServerWaitForClientTargetData* UGGA_AbilityTask_ServerWaitForClientTargetData::ServerWaitForClientTargetData(UGameplayAbility* OwningAbility, FName TaskInstanceName, bool TriggerOnce)
{
	UGGA_AbilityTask_ServerWaitForClientTargetData* MyObj = NewAbilityTask<UGGA_AbilityTask_ServerWaitForClientTargetData>(OwningAbility, TaskInstanceName);
	MyObj->bTriggerOnce = TriggerOnce;
	return MyObj;
}

void UGGA_AbilityTask_ServerWaitForClientTargetData::Activate()
{
	//  ClientPath
	if (!Ability || !Ability->GetCurrentActorInfo()->IsNetAuthority())
	{
		return;
	}

	// ServerPath
	FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
	FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
	AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UGGA_AbilityTask_ServerWaitForClientTargetData::OnTargetDataReplicatedCallback);
}

void UGGA_AbilityTask_ServerWaitForClientTargetData::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag)
{
	FGameplayAbilityTargetDataHandle MutableData = Data;
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(MutableData);
	}

	if (bTriggerOnce)
	{
		EndTask();
	}
}

void UGGA_AbilityTask_ServerWaitForClientTargetData::OnDestroy(bool AbilityEnded)
{
	if (AbilitySystemComponent.IsValid())
	{
		FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).RemoveAll(this);
	}

	Super::OnDestroy(AbilityEnded);
}
