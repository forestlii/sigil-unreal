// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/TargetingSystemTypes.h"
#include "GCS_TargetingFunctionLibrary.generated.h"

/**
 * Extended library for targeting system utilities.
 * 目标系统实用程序的扩展库。
 */
UCLASS()
class GENERICCOMBATSYSTEM_API UGCS_TargetingFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Gets the targeting source context for a targeting request handle.
	 * 获取目标请求句柄的目标源上下文。
	 * @param TargetingHandle The targeting request handle. 目标请求句柄。
	 * @return The targeting source context. 目标源上下文。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Targeting System | Targeting Types")
	static FTargetingSourceContext GetTargetingSourceContext(FTargetingRequestHandle TargetingHandle);

	/**
	 * Gets the actor targets from a targeting request handle.
	 * 从目标请求句柄获取Actor目标。
	 * @param TargetingHandle The targeting request handle. 目标请求句柄。
	 * @param Targets The actor targets (output). Actor目标（输出）。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Targeting System | Targeting Results")
	static void GetTargetingResultsActors(FTargetingRequestHandle TargetingHandle, TArray<AActor*>& Targets);

	/**
	 * Gets the hit results for a targeting handle.
	 * 获取目标句柄的命中结果。
	 * @param TargetingHandle The targeting request handle. 目标请求句柄。
	 * @param OutTargets The hit results (output). 命中结果（输出）。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Targeting System | Targeting Results")
	static void GetTargetingResults(FTargetingRequestHandle TargetingHandle, TArray<FHitResult>& OutTargets);

	/**
	 * Converts targeting location info to a source context.
	 * 将目标位置信息转换为源上下文。
	 * @param LocationInfo The targeting location info. 目标位置信息。
	 * @return The targeting source context. 目标源上下文。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Targeting System")
	static FTargetingSourceContext ConvertTargetingLocationInfoToSourceContext(FGameplayAbilityTargetingLocationInfo LocationInfo);
};