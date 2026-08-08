// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync.h"
#include "GGA_AsyncTask_WaitGameplayAbilityEnded.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGGA_AbilityEndedDelegate, const FAbilityEndedData&, AbilityEndedData);

/**
 * Given an Actor, OnAbilityEnded is triggered whenever the AbilityQuery's skill end is satisfied.
 * 给定一个Actor，只要满足AbilityQuery的技能结束，就会触发OnAbilityEnded。
 */
UCLASS()
class GENERICGAMEPLAYABILITIES_API UGGA_AsyncTask_WaitGameplayAbilityEnded : public UAbilityAsync
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "TRUE"))
	static UGGA_AsyncTask_WaitGameplayAbilityEnded* WaitGameplayAbilityEnded(AActor* TargetActor, FGameplayTagQuery AbilityQuery);

	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "TRUE"))
	static UGGA_AsyncTask_WaitGameplayAbilityEnded* WaitAbilitySpecHandleEnded(AActor* TargetActor, FGameplayAbilitySpecHandle AbilitySpecHandle);

	void HandleAbilityEnded(const FAbilityEndedData& Data);

	UPROPERTY(BlueprintAssignable)
	FGGA_AbilityEndedDelegate OnAbilityEnded;

protected:
	virtual void Activate() override;
	virtual void EndAction() override;

	FGameplayTagQuery AbilityQuery;
	FGameplayAbilitySpecHandle AbilitySpecHandle;
	FDelegateHandle DelegateHandle;
};
