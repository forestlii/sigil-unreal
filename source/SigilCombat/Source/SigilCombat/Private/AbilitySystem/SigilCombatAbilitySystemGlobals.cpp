// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "AbilitySystem/SigilCombatAbilitySystemGlobals.h"

FGameplayEffectContext* USigilCombatAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FSigilGameplayEffectContext();
}
