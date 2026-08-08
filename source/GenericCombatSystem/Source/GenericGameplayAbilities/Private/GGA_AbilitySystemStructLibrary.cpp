// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GGA_AbilitySystemStructLibrary.h"


bool FGGA_GameplayEffectContainerSpec::HasValidEffects() const
{
	return TargetGameplayEffectSpecs.Num() > 0;
}

bool FGGA_GameplayEffectContainerSpec::HasValidTargets() const
{
	return TargetData.Num() > 0;
}

void FGGA_GameplayEffectContainerSpec::AddTargets(const TArray<FHitResult>& HitResults, const TArray<AActor*>& TargetActors)
{
	for (const FHitResult& HitResult : HitResults)
	{
		FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
		TargetData.Add(NewData);
	}

	if (TargetActors.Num() > 0)
	{
		FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
		NewData->TargetActorArray.Append(TargetActors);
		TargetData.Add(NewData);
	}
}
