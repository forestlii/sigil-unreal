// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GGA_TargetType.generated.h"

/**
* Deprecated
*/
UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class GENERICGAMEPLAYABILITIES_API UGGA_TargetType : public UObject
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	UGGA_TargetType()
	{
	}

	/** Called to determine targets to apply gameplay effects to */
	UFUNCTION(BlueprintNativeEvent)
	void GetTargets(AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const;
	virtual void GetTargets_Implementation(AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const;
};
