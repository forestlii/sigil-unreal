// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Utilities/GGA_GameplayAbilityTargetDataFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"

FGameplayAbilityTargetDataHandle UGGA_GameplayAbilityTargetDataFunctionLibrary::AbilityTargetDataFromHitResults(const TArray<FHitResult>& HitResults, bool OneTargetPerHandle)
{
	// Construct TargetData
	if (OneTargetPerHandle)
	{
		FGameplayAbilityTargetDataHandle Handle;
		for (int32 i = 0; i < HitResults.Num(); ++i)
		{
			if (::IsValid(HitResults[i].GetActor()))
			{
				FGameplayAbilityTargetDataHandle TempHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(HitResults[i]);
				Handle.Append(TempHandle);
			}
		}
		return Handle;
	}
	else
	{
		FGameplayAbilityTargetDataHandle Handle;

		for (const FHitResult& HitResult : HitResults)
		{
			FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
			Handle.Add(NewData);
		}

		return Handle;
	}
}

void UGGA_GameplayAbilityTargetDataFunctionLibrary::AddTargetDataToContext(FGameplayAbilityTargetDataHandle TargetData, FGameplayEffectContextHandle EffectContext)
{
	for (auto Data : TargetData.Data)
	{
		if (Data.IsValid())
		{
			Data->AddTargetDataToContext(EffectContext, true);
		}
	}
}
