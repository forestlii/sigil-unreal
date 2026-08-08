// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayBehaviorConfig.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "SigilGameplayBehaviorConfig_InteractionWithAbility.generated.h"

class UGameplayAbility;
class UUserWidget;

/**
 * Configuration for ability-based interaction behavior.
 * 基于技能的交互行为配置。
 */
UCLASS(DisplayName="Gameplay Behavior Config Interaction (GGS)")
class SIGILINTERACTION_API USigilGameplayBehaviorConfig_InteractionWithAbility : public UGameplayBehaviorConfig
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the interaction behavior config.
	 * 交互行为配置构造函数。
	 */
	USigilGameplayBehaviorConfig_InteractionWithAbility();

	/**
	 * The ability to grant and activate when interaction begins.
	 * 交互开始时赋予并激活的技能。
	 * @note Must be instanced and not LocalOnly. Does not support event-triggered abilities.
	 * @注意 必须是实例化的技能，不能是LocalOnly。不支持事件触发的技能。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	TSoftClassPtr<UGameplayAbility> AbilityToGrant;

	/**
	 * The level of the ability, used for visual distinctions.
	 * 技能等级，用于视觉区分。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	int32 AbilityLevel{0};

#if WITH_EDITORONLY_DATA
	/**
	 * Validates data in the editor.
	 * 在编辑器中验证数据。
	 * @param Context The data validation context. 数据验证上下文。
	 * @return The validation result. 验证结果。
	 */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
