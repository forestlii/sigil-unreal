// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GGA_TargetType.h"
#include "GGA_TargetType_UseEventData.generated.h"

/** Trivial target type that pulls the target out of the event data */
UCLASS(NotBlueprintable)
class GENERICGAMEPLAYABILITIES_API UGGA_TargetType_UseEventData : public UGGA_TargetType
{
	GENERATED_BODY()
public:
	// Constructor and overrides
	UGGA_TargetType_UseEventData() {}

	/** Uses the passed in event data */
	virtual void GetTargets_Implementation(AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};
