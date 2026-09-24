// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"


namespace SigilAbilityActivateFailTags
{
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown)
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cost)
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TagsBlocked);
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TagsMissing);
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Networking);
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ActivationGroup);
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SourceObjectInactive);

	/**
	 * Failures for required context an ability could not resolve at activation time.
	 * 激活时所需上下文缺失导致的失败原因。
	 */
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Missing_AbilityAction);
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Missing_AttackInstigator);
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Missing_MovementInput);
}

namespace SigilAbilityTraitTags
{
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ActivationOnSpawn)
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Persistent)
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MovementCancellable)

}

namespace SigilStateTags
{
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interacting)
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InteractingRemoval)
}

namespace SigilSetByCallerTags
{
	/** Default SetByCaller data tag USigilGameplayAbility writes its CooldownDuration into on a shared cooldown effect. */
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CooldownDuration)
}

namespace SigilCooldownTags
{
	/**
	 * Marker a shared cooldown GameplayEffect grants so it passes the engine's IsDataValid rule ("a cooldown GE must grant
	 * tags"). USigilGameplayAbility::GetCooldownTags() strips it from the union when CooldownTags is set, so abilities that
	 * share the effect never block each other through it.
	 * 共享冷却 GameplayEffect 授予的标记标签，用于通过引擎 IsDataValid 的"冷却 GE 必须授予标签"校验。
	 * 配置了 CooldownTags 时 USigilGameplayAbility::GetCooldownTags() 会把它从并集里剔除，共享同一 GE 的技能不会因它互相阻塞。
	 */
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SharedMarker)
}
