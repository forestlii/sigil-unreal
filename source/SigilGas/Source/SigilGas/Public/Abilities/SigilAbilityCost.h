// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "SigilAbilityCost.generated.h"

/**
 * Base class for defining costs associated with a gameplay ability (e.g., ammo, charges).
 * 定义与游戏技能相关成本的基类（例如弹药、次数）。
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew, Abstract)
class SIGILGAS_API USigilAbilityCost : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the ability cost.
	 * 技能成本构造函数。
	 */
	USigilAbilityCost()
	{
	}

	/**
	 * Checks if the ability cost can be afforded.
	 * 检查是否能支付技能成本。
	 * @param Ability The gameplay ability. 游戏技能。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param OptionalRelevantTags Tags for failure reasons (optional, output). 失败原因标签（可选，输出）。
	 * @return True if the cost can be paid, false otherwise. 如果成本可支付则返回true，否则返回false。
	 */
	virtual bool CheckCost(const UGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const;

	/**
	 * Applies the ability cost to the target.
	 * 将技能成本应用于目标。
	 * @param Ability The gameplay ability. 游戏技能。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param ActivationInfo The activation info. 激活信息。
	 */
	virtual void ApplyCost(const UGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

	/**
	 * Checks if the cost should only be applied on a successful hit.
	 * 检查成本是否仅在成功命中时应用。
	 * @return True if the cost applies only on hit, false otherwise. 如果成本仅在命中时应用则返回true，否则返回false。
	 */
	bool ShouldOnlyApplyCostOnHit() const { return bOnlyApplyCostOnHit; }

protected:
	/**
	 * Blueprint event for checking the ability cost.
	 * 检查技能成本的蓝图事件。
	 * @param Ability The gameplay ability. 游戏技能。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param OptionalRelevantTags Tags for failure reasons. 失败原因标签。
	 * @return True if the cost can be paid, false otherwise. 如果成本可支付则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category=Costs, meta=(DisplayName="Check Cost"))
	bool BlueprintCheckCost(const UGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo,
	                        const FGameplayTagContainer& OptionalRelevantTags) const;

	/**
	 * Blueprint event for applying the ability cost.
	 * 应用技能成本的蓝图事件。
	 * @param Ability The gameplay ability. 游戏技能。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param ActivationInfo The activation info. 激活信息。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category=Costs, meta=(DisplayName="Apply Cost"))
	void BlueprintApplyCost(const UGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo& ActivationInfo);

	/**
	 * Determines if the cost applies only on a successful hit.
	 * 确定成本是否仅在成功命中时应用。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	bool bOnlyApplyCostOnHit = false;
};
