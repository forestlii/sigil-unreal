// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilAbilitySystemEnumLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "UObject/Interface.h"
#include "SigilGameplayAbilityInterface.generated.h"

/**
 * Interface for gameplay abilities to integrate with SigilAbilitySystemComponent.
 * 与SigilAbilitySystemComponent集成的游戏技能接口。
 */
UINTERFACE(MinimalAPI, BlueprintType, meta=(CannotImplementInterfaceInBlueprint))
class USigilGameplayAbilityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implementation class for gameplay ability interface.
 * 游戏技能接口的实现类。
 */
class SIGILGAS_API ISigilGameplayAbilityInterface
{
	GENERATED_BODY()

public:
	/**
	 * Retrieves the activation group for the ability.
	 * 获取技能的激活组。
	 * @return The activation group. 激活组。
	 */
	UFUNCTION(BlueprintCallable, Category="Ability")
	virtual ESigilAbilityActivationGroup GetActivationGroup() const = 0;

	/**
	 * Sets the activation group for the ability.
	 * 设置技能的激活组。
	 * @param NewGroup The new activation group. 新激活组。
	 */
	UFUNCTION(BlueprintCallable, Category="Ability")
	virtual void SetActivationGroup(ESigilAbilityActivationGroup NewGroup) = 0;

	/**
	 * Attempts to activate the ability on spawn.
	 * 尝试在生成时激活技能。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param Spec The ability spec. 技能规格。
	 */
	virtual void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const = 0;

	/**
	 * Handles ability activation failure.
	 * 处理技能激活失败。
	 * @param FailureReason The reason for failure. 失败原因。
	 */
	virtual void HandleActivationFailed(const FGameplayTagContainer& FailureReason) const = 0;

	/**
	 * Attempts to activate the ability with batched RPCs.
	 * 尝试使用批处理RPC激活技能。
	 * @param InAbilityHandle The ability handle. 技能句柄。
	 * @param EndAbilityImmediately Whether to end the ability immediately. 是否立即结束技能。
	 * @return True if activation was successful, false otherwise. 如果激活成功则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Net")
	virtual bool BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately) = 0;

	/**
	 * Ends the ability externally.
	 * 外部结束技能。
	 */
	UFUNCTION(BlueprintCallable, Category="Ability")
	virtual void ExternalEndAbility() = 0;
};