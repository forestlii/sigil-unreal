// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilAbilitySystemEnumLibrary.generated.h"

/**
 * Enum defining how an ability activates relative to other abilities.
 * 定义技能相对于其他技能的激活方式的枚举。
 */
UENUM(BlueprintType)
enum class ESigilAbilityActivationGroup : uint8
{
	/**
	 * Ability runs independently of other abilities.
	 * 技能独立于其他技能运行。
	 */
	Independent,

	/**
	 * Ability is canceled and replaced by other exclusive abilities.
	 * 技能被其他独占技能取消并替换。
	 */
	Exclusive_Replaceable,

	/**
	 * Ability blocks all other exclusive abilities from activating.
	 * 技能阻止其他独占技能激活。
	 */
	Exclusive_Blocking,

	/**
	 * Maximum value (hidden).
	 * 最大值（隐藏）。
	 */
	MAX UMETA(Hidden)
};