// Copyright 2020 Dan Kestranek. Licensed under the MIT License (GASDocumentation: https://github.com/tranek/GASDocumentation).
// Modifications Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Async/AbilityAsync.h"
#include "SigilAsyncTask_EffectStackChanged.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSigilOnGameplayEffectStackChanged, FGameplayTag, EffectGameplayTag, FActiveGameplayEffectHandle, Handle, int32, NewStackCount, int32, OldStackCount);

/**
 * Blueprint node that listens for stack count changes of a GameplayEffect identified by one of its asset or granted tags.
 * Broadcasts (1, 0) when the effect is first applied, (New, Old) on each stack change and (0, Old) when it is removed.
 * Intended for UMG (call EndAction from Destruct).
 * Ported from GASDocumentation UAsyncTaskEffectStackChanged and rebased on UAbilityAsync.
 * 监听以资产 / 授予标签识别的 GameplayEffect 的堆叠数变化：首次应用广播 (1, 0)，每次堆叠变化广播 (New, Old)，移除时广播 (0, Old)。
 * 用于 UMG（在 Destruct 里调用 EndAction）。
 */
UCLASS()
class SIGILGAS_API USigilAsyncTask_EffectStackChanged : public UAbilityAsync
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FSigilOnGameplayEffectStackChanged OnGameplayEffectStackChange;

	/**
	 * Listens for stack changes of the GameplayEffect carrying EffectGameplayTag as an asset or granted tag.
	 * 监听携带 EffectGameplayTag（资产或授予标签）的 GameplayEffect 的堆叠变化。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (BlueprintInternalUseOnly = "true"))
	static USigilAsyncTask_EffectStackChanged* ListenForGameplayEffectStackChange(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag EffectGameplayTag);

	virtual void Activate() override;
	virtual void EndAction() override;

protected:

	virtual void OnActiveGameplayEffectAddedCallback(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);
	virtual void OnRemoveGameplayEffectCallback(const FActiveGameplayEffect& EffectRemoved);
	virtual void GameplayEffectStackChanged(FActiveGameplayEffectHandle EffectHandle, int32 NewStackCount, int32 PreviousStackCount);

	bool SpecMatchesEffectTag(const FGameplayEffectSpec& Spec) const;

	FGameplayTag EffectGameplayTag;
	FActiveGameplayEffectHandle ActiveEffectHandle;
};
