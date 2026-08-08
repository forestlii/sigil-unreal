// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GGA_GameplayAbilityTargetDataFunctionLibrary.generated.h"

/**
 * Blueprint function library for gameplay ability target data operations.
 * 用于游戏技能目标数据操作的蓝图函数库。
 */
UCLASS()
class GENERICGAMEPLAYABILITIES_API UGGA_GameplayAbilityTargetDataFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Creates a target data handle from hit results.
	 * 从命中结果创建目标数据句柄。
	 * @param HitResults Array of hit results. 命中结果数组。
	 * @param OneTargetPerHandle Whether to create one target per handle. 是否每个句柄一个目标。
	 * @return The target data handle. 目标数据句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|TargetData")
	static FGameplayAbilityTargetDataHandle AbilityTargetDataFromHitResults(const TArray<FHitResult>& HitResults, bool OneTargetPerHandle);

	/**
	 * Adds target data to an effect context.
	 * 将目标数据添加到效果上下文。
	 * @param TargetData The target data to add. 要添加的目标数据。
	 * @param EffectContext The effect context to modify. 要修改的效果上下文。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|TargetData")
	static void AddTargetDataToContext(UPARAM(ref) FGameplayAbilityTargetDataHandle TargetData, UPARAM(ref) FGameplayEffectContextHandle EffectContext);
};