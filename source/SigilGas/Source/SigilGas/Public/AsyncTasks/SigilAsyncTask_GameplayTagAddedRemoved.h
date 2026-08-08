// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Abilities/Async/AbilityAsync.h"
#include "SigilAsyncTask_GameplayTagAddedRemoved.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameplayTagAddedRemoved, FGameplayTag, Tag);

/**
 * 蓝图节点，用于监听AbilitySystemComponent上的标记添加/移除变化。
 * 一般用于蓝图/UMG
 */
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = AsyncTask))
class SIGILGAS_API USigilAsyncTask_GameplayTagAddedRemoved : public UAbilityAsync
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnGameplayTagAddedRemoved OnTagAdded;

	UPROPERTY(BlueprintAssignable)
	FOnGameplayTagAddedRemoved OnTagRemoved;

	// Listens for FGameplayTags added and removed.
	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (BlueprintInternalUseOnly = "true"))
	static USigilAsyncTask_GameplayTagAddedRemoved* ListenForGameplayTagAddedOrRemoved(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer Tags);

	/**
	 * You must call this function manually when you want the AsyncTask to end. For UMG Widgets, you would call it in the Widget's Destruct event.
	 * 要结束 AsyncTask 时，必须手动调用该函数。对于 UMG Widget，您可以在 Widget 的 Destruct 事件中调用该函数。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta=(DeprecatedFunction, DeprecationMessage="Use EndAction"))
	void EndTask();

protected:
	virtual void EndAction() override;

	FGameplayTagContainer Tags;

	virtual void TagChanged(const FGameplayTag Tag, int32 NewCount);
};
