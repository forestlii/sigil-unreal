// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilAbilitySystemStructLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SigilGameplayEffectContainerFunctionLibrary.generated.h"

/**
 * Blueprint function library for gameplay effect container operations.
 * 用于游戏效果容器操作的蓝图函数库。
 */
UCLASS()
class SIGILGAS_API USigilGameplayEffectContainerFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Checks if the gameplay effect container is valid.
	 * 检查游戏效果容器是否有效。
	 * @param Container The gameplay effect container. 游戏效果容器。
	 * @return True if the container is valid, false otherwise. 如果容器有效则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayEffect|Container")
	static bool IsValidContainer(const FSigilGameplayEffectContainer& Container);

	/**
	 * Checks if the container spec has valid effects.
	 * 检查容器规格是否具有有效效果。
	 * @param ContainerSpec The gameplay effect container spec. 游戏效果容器规格。
	 * @return True if the spec has valid effects, false otherwise. 如果规格有有效效果则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayEffect|Container")
	static bool HasValidEffects(const FSigilGameplayEffectContainerSpec& ContainerSpec);

	/**
	 * Checks if the container spec has valid targets.
	 * 检查容器规格是否具有有效目标。
	 * @param ContainerSpec The gameplay effect container spec. 游戏效果容器规格。
	 * @return True if the spec has valid targets, false otherwise. 如果规格有有效目标则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayEffect|Container")
	static bool HasValidTargets(const FSigilGameplayEffectContainerSpec& ContainerSpec);

	/**
	 * Adds targets to a copy of the effect container spec.
	 * 将目标添加到效果容器规格的副本。
	 * @param ContainerSpec The gameplay effect container spec. 游戏效果容器规格。
	 * @param HitResults Array of hit results to add. 要添加的命中结果数组。
	 * @param TargetActors Array of target actors to add. 要添加的目标演员数组。
	 * @return The modified container spec. 修改后的容器规格。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayEffect|Container", meta = (AutoCreateRefTerm = "HitResults,TargetActors"))
	static FSigilGameplayEffectContainerSpec AddTargets(const FSigilGameplayEffectContainerSpec& ContainerSpec, const TArray<FHitResult>& HitResults,
	                                                   const TArray<AActor*>& TargetActors);

	/**
	 * Creates a gameplay effect container spec from a container and event data.
	 * 从容器和事件数据创建游戏效果容器规格。
	 * @param Container The gameplay effect container. 游戏效果容器。
	 * @param EventData Event data for creating the spec. 用于创建规格的事件数据。
	 * @param OverrideGameplayLevel Optional override for gameplay effect level. 可选的游戏效果等级覆盖。
	 * @param SourceAbility Optional ability to derive source data. 可选的技能以获取源数据。
	 * @return The created container spec. 创建的容器规格。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayEffect|Container", meta=(AutoCreateRefTerm = "EventData"))
	static FSigilGameplayEffectContainerSpec MakeEffectContainerSpec(UPARAM(ref) const FSigilGameplayEffectContainer& Container, const FGameplayEventData& EventData,
	                                                                int32 OverrideGameplayLevel = -1, UGameplayAbility* SourceAbility = nullptr);

	/**
	 * Applies a gameplay effect container spec.
	 * 应用游戏效果容器规格。
	 * @param ExecutingAbility The ability executing the spec. 执行规格的技能。
	 * @param ContainerSpec The container spec to apply. 要应用的容器规格。
	 * @return Array of active gameplay effect handles. 活动游戏效果句柄数组。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Ability|EffectContainer",meta=(DefaultToSelf="ExecutingAbility"))
	static TArray<FActiveGameplayEffectHandle> ApplyEffectContainerSpec(UGameplayAbility* ExecutingAbility, const FSigilGameplayEffectContainerSpec& ContainerSpec);

	/**
	 * Applies a gameplay effect container spec to all targets in its target data.
	 * 将游戏效果容器规格应用于其目标数据中的所有目标。
	 * @param ContainerSpec The container spec to apply. 要应用的容器规格。
	 * @return Array of active gameplay effect handles. 活动游戏效果句柄数组。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayEffect|Container")
	static TArray<FActiveGameplayEffectHandle> ApplyExternalEffectContainerSpec(const FSigilGameplayEffectContainerSpec& ContainerSpec);
};