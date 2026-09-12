// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Abilities/SigilAbilityCost.h"
#include "SigilAbilityCost_ItemIntegerAttribute.generated.h"

/** 从技能来源武器的物品实例支付整数属性成本；预测端只检查，由拥有武器的 authority 扣除。 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class SIGILARSENAL_API USigilAbilityCost_ItemIntegerAttribute : public USigilAbilityCost
{
	GENERATED_BODY()

public:
	USigilAbilityCost_ItemIntegerAttribute();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Costs")
	FGameplayTag Tag;

	/** 必须大于零；无需浮点缩放或舍入规则。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Costs", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Costs")
	FGameplayTag FailureTag;

	virtual bool CheckCost(const UGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const UGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo) override;
};
