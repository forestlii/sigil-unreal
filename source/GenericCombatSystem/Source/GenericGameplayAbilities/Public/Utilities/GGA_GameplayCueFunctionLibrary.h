// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GGA_GameplayCueFunctionLibrary.generated.h"

/**
 * Blueprint function library for gameplay cue operations.
 * 用于游戏提示操作的蓝图函数库。
 */
UCLASS()
class GENERICGAMEPLAYABILITIES_API UGGA_GameplayCueFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Executes a local gameplay cue on an actor.
	 * 在演员上执行本地游戏提示。
	 * @param Actor The actor to execute the cue on. 要执行提示的演员。
	 * @param GameplayCueTag The gameplay cue tag. 游戏提示标签。
	 * @param GameplayCueParameters Parameters for the gameplay cue. 游戏提示参数。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayCue", Meta = (DefaultToSelf="Actor", AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	static void ExecuteGameplayCueLocal(AActor* Actor, const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	/**
	 * Adds a local gameplay cue to an actor.
	 * 向演员添加本地游戏提示。
	 * @param Actor The actor to add the cue to. 要添加提示的演员。
	 * @param GameplayCueTag The gameplay cue tag. 游戏提示标签。
	 * @param GameplayCueParameters Parameters for the gameplay cue. 游戏提示参数。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayCue", Meta = (DefaultToSelf="Actor", AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	static void AddGameplayCueLocal(AActor* Actor, const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	/**
	 * Removes a local gameplay cue from an actor.
	 * 从演员移除本地游戏提示。
	 * @param Actor The actor to remove the cue from. 要移除提示的演员。
	 * @param GameplayCueTag The gameplay cue tag. 游戏提示标签。
	 * @param GameplayCueParameters Parameters for the gameplay cue. 游戏提示参数。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayCue", Meta = (DefaultToSelf="Actor", AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	static void RemoveGameplayCueLocal(AActor* Actor, const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);
};