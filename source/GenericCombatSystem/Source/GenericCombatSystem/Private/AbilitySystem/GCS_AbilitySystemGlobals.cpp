// Copyright 2025 RedMoonGames All Rights Reserved.


#include "AbilitySystem/GCS_AbilitySystemGlobals.h"

FGameplayEffectContext* UGCS_AbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FGCS_GameplayEffectContext();
}
