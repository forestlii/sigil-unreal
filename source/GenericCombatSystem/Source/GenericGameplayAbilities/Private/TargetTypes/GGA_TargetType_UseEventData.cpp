// Copyright 2025 RedMoonGames All Rights Reserved.


#include "TargetTypes/GGA_TargetType_UseEventData.h"

#include "AbilitySystemBlueprintLibrary.h"


void UGGA_TargetType_UseEventData::GetTargets_Implementation(AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const
{
	const FHitResult* FoundHitResult = EventData.ContextHandle.GetHitResult();
	const FHitResult TargetDataHitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(EventData.TargetData, 0);

	if (FoundHitResult)
	{
		OutHitResults.Add(*FoundHitResult);
	}
	else if (TargetDataHitResult.IsValidBlockingHit())
	{
		OutHitResults.Add(TargetDataHitResult);
	}
	else if (EventData.Target)
	{
		const AActor* Actor = EventData.Target;
		OutActors.Add(const_cast<AActor*>(Actor));
	}
}
