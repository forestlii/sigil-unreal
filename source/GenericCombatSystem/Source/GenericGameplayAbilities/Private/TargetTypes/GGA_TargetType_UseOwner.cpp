// Copyright 2025 RedMoonGames All Rights Reserved.


#include "TargetTypes/GGA_TargetType_UseOwner.h"


void UGGA_TargetType_UseOwner::GetTargets_Implementation(AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const
{
	OutActors.Add(TargetingActor);
}
