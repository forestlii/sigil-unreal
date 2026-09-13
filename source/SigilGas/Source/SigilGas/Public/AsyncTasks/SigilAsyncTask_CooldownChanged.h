// Copyright 2020 Dan Kestranek. Licensed under the MIT License (GASDocumentation: https://github.com/tranek/GASDocumentation).
// Modifications Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Async/AbilityAsync.h"
#include "GameplayTagContainer.h"
#include "SigilAsyncTask_CooldownChanged.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSigilOnCooldownChanged, FGameplayTag, CooldownTag, float, TimeRemaining, float, Duration);

/**
 * Blueprint node that listens for cooldown GameplayEffects beginning and ending on an ability system component, keyed
 * by the cooldown tags. Intended for UMG (call EndAction from Destruct).
 * Ported from GASDocumentation UAsyncTaskCooldownChanged and rebased on UAbilityAsync.
 * Known limitation: on an ASC in Minimal replication mode the client never receives the cooldown GE, so OnCooldownBegin
 * will not fire there (tag events still do).
 * 监听技能系统组件上冷却 GameplayEffect 的开始 / 结束（按冷却标签）。用于 UMG（在 Destruct 里调用 EndAction）。
 * 已知限制：Minimal 复制模式下客户端收不到冷却 GE，OnCooldownBegin 不会触发（标签事件仍会）。
 */
UCLASS()
class SIGILGAS_API USigilAsyncTask_CooldownChanged : public UAbilityAsync
{
	GENERATED_BODY()

public:
	/** A cooldown effect carrying one of the listened tags was applied. 带监听标签之一的冷却效果已应用。 */
	UPROPERTY(BlueprintAssignable)
	FSigilOnCooldownChanged OnCooldownBegin;

	/** One of the listened cooldown tags dropped to zero count. TimeRemaining / Duration are -1. 监听的冷却标签计数归零；TimeRemaining / Duration 为 -1。 */
	UPROPERTY(BlueprintAssignable)
	FSigilOnCooldownChanged OnCooldownEnd;

	/**
	 * Listens for changes (Begin and End) to cooldown GameplayEffects based on the cooldown tags.
	 * bUseServerCooldown determines if the server's cooldown is reported in addition to the locally predicted one.
	 * When using the server cooldown, TimeRemaining and Duration return -1 to signal that the local predicted cooldown began.
	 * 按冷却标签监听冷却 GameplayEffect 的开始与结束。bUseServerCooldown 决定是否在本地预测冷却之外也报告服务器冷却；
	 * 使用服务器冷却时，本地预测冷却开始会以 TimeRemaining / Duration = -1 通知。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (BlueprintInternalUseOnly = "true"))
	static USigilAsyncTask_CooldownChanged* ListenForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer CooldownTags, bool bUseServerCooldown);

	virtual void Activate() override;
	virtual void EndAction() override;

protected:

	virtual void OnActiveGameplayEffectAddedCallback(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);
	virtual void CooldownTagChanged(const FGameplayTag CooldownTag, int32 NewCount);

	bool GetCooldownRemainingForTag(const FGameplayTagContainer& InCooldownTags, float& TimeRemaining, float& CooldownDuration) const;

	FGameplayTagContainer CooldownTags;
	bool bUseServerCooldown = false;
};
