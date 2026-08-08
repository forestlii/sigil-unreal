// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Utilities/SigilGameplayAbilityTargetDataFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"

FGameplayAbilityTargetDataHandle USigilGameplayAbilityTargetDataFunctionLibrary::AbilityTargetDataFromHitResults(const TArray<FHitResult>& HitResults, bool OneTargetPerHandle)
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

void USigilGameplayAbilityTargetDataFunctionLibrary::AddTargetDataToContext(FGameplayAbilityTargetDataHandle TargetData, FGameplayEffectContextHandle EffectContext)
{
	for (auto Data : TargetData.Data)
	{
		if (Data.IsValid())
		{
			Data->AddTargetDataToContext(EffectContext, true);
		}
	}
}
