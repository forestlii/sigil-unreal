// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GGA_AbilitySystemGlobals.h"
#include "GCS_GameplayEffectContext.h"
#include "GCS_AbilitySystemGlobals.generated.h"

UCLASS()
class GENERICCOMBATSYSTEM_API UGCS_AbilitySystemGlobals : public UGGA_AbilitySystemGlobals
{
	GENERATED_BODY()

public:
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
