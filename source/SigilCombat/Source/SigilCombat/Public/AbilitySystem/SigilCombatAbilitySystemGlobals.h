// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilAbilitySystemGlobals.h"
#include "SigilGameplayEffectContext.h"
#include "SigilCombatAbilitySystemGlobals.generated.h"

UCLASS()
class SIGILCOMBAT_API USigilCombatAbilitySystemGlobals : public USigilAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
