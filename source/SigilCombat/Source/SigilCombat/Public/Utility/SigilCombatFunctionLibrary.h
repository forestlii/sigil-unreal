// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilCombatEnumLibrary.h"
#include "SigilCombatStructLibrary.h"
#include "CombatFlow/SigilAttackDefinition.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SigilCombatFunctionLibrary.generated.h"

class USigilAttackRequest_Base;
class ISigilCombatTeamAgentInterface;
class ISigilWeaponInterface;
class ISigilCombatInterface;

/**
 * Utility functions for combat-related operations.
 * 战斗相关操作的实用函数。
 */
UCLASS()
class SIGILCOMBAT_API USigilCombatFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Gets the combat team agent interface from an actor or its components.
	 * 从Actor或其组件获取战斗队伍代理接口。
	 * @param Actor The actor to query. 要查询的Actor。
	 * @return The combat team agent interface. 战斗队伍代理接口。
	 */
	UFUNCTION(BlueprintCallable, Category="GCS|CombatTeam", meta=(DefaultToSelf="Actor"))
	static TScriptInterface<ISigilCombatTeamAgentInterface> GetCombatTeamAgentInterface(AActor* Actor);

	/**
	 * Finds the combat team agent interface on an actor or its components.
	 * 在Actor或其组件上查找战斗队伍代理接口。
	 * @param Actor The actor to query. 要查询的Actor。
	 * @param OutInterface The found interface (output). 找到的接口（输出）。
	 * @return True if found. 如果找到返回true。
	 */
	UFUNCTION(BlueprintCallable, Category="GCS|CombatTeam", meta=(DefaultToSelf="Actor", ExpandBoolAsExecs="ReturnValue"))
	static bool FindCombatTeamAgentInterface(AActor* Actor, TScriptInterface<ISigilCombatTeamAgentInterface>& OutInterface);

	/**
	 * Gets the combat interface from an actor or its components.
	 * 从Actor或其组件获取战斗接口。
	 * @param Actor The actor to query. 要查询的Actor。
	 * @return The combat interface. 战斗接口。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS", meta=(DefaultToSelf="Actor"))
	static TScriptInterface<ISigilCombatInterface> GetCombatInterface(AActor* Actor);

	/**
	 * Gets the implementer of the combat interface.
	 * 获取战斗接口的实现者。
	 * @param Actor The actor to query. 要查询的Actor。
	 * @return The object implementing the combat interface. 实现战斗接口的对象。
	 */
	static UObject* GetCombatInterfaceImplementer(AActor* Actor);

	/**
	 * Gets the weapon interface from an actor or its components.
	 * 从Actor或其组件获取武器接口。
	 * @param Actor The actor to query. 要查询的Actor。
	 * @return The weapon interface. 武器接口。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS", meta=(DefaultToSelf="Actor"))
	static TScriptInterface<ISigilWeaponInterface> GetWeaponInterface(AActor* Actor);

	/**
	 * Gets the main skeletal mesh component from an actor.
	 * 从Actor获取主要骨骼网格组件。
	 * @param Actor The actor to query. 要查询的Actor。
	 * @param OverrideMeshLookupTag Optional override tag for lookup. 可选的覆盖查找标签。
	 * @return The skeletal mesh component. 骨骼网格组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS", meta=(DefaultToSelf="Actor"))
	static USkeletalMeshComponent* GetMainCharacterMeshComponent(AActor* Actor, FName OverrideMeshLookupTag = NAME_None);

	/**
	 * Gets the main mesh component from an actor.
	 * 从Actor获取主要网格组件。
	 * @param Actor The actor to query. 要查询的Actor。
	 * @param OverrideMeshLookupTag Optional override tag for lookup. 可选的覆盖查找标签。
	 * @return The mesh component. 网格组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS", meta=(DefaultToSelf="Actor"))
	static UMeshComponent* GetMainMeshComponent(AActor* Actor, FName OverrideMeshLookupTag = NAME_None);

	/**
	 * Gets socket names with a specified prefix from a component.
	 * 从组件获取具有指定前缀的插槽名称。
	 * @param Component The component to query. 要查询的组件。
	 * @param Prefix The prefix to match. 要匹配的前缀。
	 * @param SearchCase The case sensitivity for the search. 搜索的大小写敏感性。
	 * @return Array of matching socket names. 匹配的插槽名称数组。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS")
	static TArray<FName> GetSocketNamesWithPrefix(const USceneComponent* Component, FString Prefix, ESearchCase::Type SearchCase);

	/**
	 * Finds the combat interface on an actor or its components.
	 * 在Actor或其组件上查找战斗接口。
	 * @param Actor The actor to query. 要查询的Actor。
	 * @param OutInterface The found interface (output). 找到的接口（输出）。
	 * @return True if found. 如果找到返回true。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS", meta=(DefaultToSelf="Actor", ExpandBoolAsExecs="ReturnValue"))
	static bool FindCombatInterface(AActor* Actor, TScriptInterface<ISigilCombatInterface>& OutInterface);

	/**
	 * Finds the weapon interface on an actor or its components.
	 * 在Actor或其组件上查找武器接口。
	 * @param Actor The actor to query. 要查询的Actor。
	 * @param OutInterface The found interface (output). 找到的接口（输出）。
	 * @return True if found. 如果找到返回true。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS", meta=(DefaultToSelf="Actor", ExpandBoolAsExecs="ReturnValue"))
	static bool FindWeaponInterface(AActor* Actor, TScriptInterface<ISigilWeaponInterface>& OutInterface);

	/**
	 * Calculates the angle between two actors.
	 * 计算两个Actor之间的角度。
	 * @param From The source actor. 来源Actor。
	 * @param To The target actor. 目标Actor。
	 * @return The angle as a rotator. 角度（旋转器）。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS")
	static FRotator CalculateAngleBetweenActors(const AActor* From, const AActor* To);

	/**
	 * Determines the direction from an angle.
	 * 从角度确定方向。
	 * @param Angle The input angle. 输入角度。
	 * @return The direction enum. 方向枚举。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS")
	static ESigilDirection CalculateDirectionFromAngle(const float Angle);

	/**
	 * Selects a montage based on direction.
	 * 根据方向选择蒙太奇。
	 * @param Direction The direction enum. 方向枚举。
	 * @param Montages Array of montages to choose from. 可选的蒙太奇数组。
	 * @return The selected montage. 选中的蒙太奇。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS")
	static TSoftObjectPtr<UAnimMontage> SelectMontageByDirection(ESigilDirection Direction, TArray<TSoftObjectPtr<UAnimMontage>> Montages);

	/**
	 * Adds a tagged value to an array.
	 * 向数组添加标记值。
	 * @param TaggedValues The tagged values array (modified). 标记值数组（修改）。
	 * @param Tag The gameplay tag. 游戏标签。
	 * @param ValueToAdd The value to add. 要添加的值。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="GCS")
	static void AddTaggedValue(UPARAM(ref) TArray<FSigilTaggedValue>& TaggedValues, FGameplayTag Tag, float ValueToAdd);

	/**
	 * Gets a tagged value from an array.
	 * 从数组获取标记值。
	 * @param TaggedValues The tagged values array. 标记值数组。
	 * @param Tag The gameplay tag to find. 要查找的游戏标签。
	 * @return The associated value. 关联值。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="GCS")
	static float GetTaggedValue(const TArray<FSigilTaggedValue> TaggedValues, FGameplayTag Tag);

	/**
	 * Filters a gameplay tag container based on another container.
	 * 根据另一个容器过滤游戏标签容器。
	 * @param TagContainer The container to filter. 要过滤的容器。
	 * @param OtherContainer The container to filter against. 过滤依据的容器。
	 * @return The filtered tag container. 过滤后的标签容器。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="GCS")
	static FGameplayTagContainer FilterGameplayTagContainer(const FGameplayTagContainer& TagContainer, FGameplayTagContainer OtherContainer);

	/**
	 * Adds an attack definition handle to a gameplay effect spec.
	 * 将攻击定义句柄添加到游戏效果规格。
	 * @param SpecHandle The effect spec handle. 效果规格句柄。
	 * @param AttackHandle The attack definition handle. 攻击定义句柄。
	 * @return The modified effect spec handle. 修改后的效果规格句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="GCS")
	static FGameplayEffectSpecHandle AddAttackHandleToGameplayEffectSpec(FGameplayEffectSpecHandle SpecHandle, FDataTableRowHandle AttackHandle);

	/**
	 * Adds an attack definition to a gameplay effect spec.
	 * 将攻击定义添加到游戏效果规格。
	 * @param SpecHandle The effect spec handle. 效果规格句柄。
	 * @param AtkDefinition The attack definition. 攻击定义。
	 * @return The modified effect spec handle. 修改后的效果规格句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="GCS")
	static FGameplayEffectSpecHandle AddAttackDefinitionToGameplayEffectSpec(FGameplayEffectSpecHandle SpecHandle, const FSigilAttackDefinition& AtkDefinition);

	/**
	 * Adds an attack definition handle to a gameplay effect container spec.
	 * 将攻击定义句柄添加到游戏效果容器规格。
	 * @param ContainerSpec The effect container spec. 效果容器规格。
	 * @param AttackHandle The attack definition handle. 攻击定义句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="GCS")
	static void AddAttackHandleToGameplayEffectContainerSpec(FSigilGameplayEffectContainerSpec ContainerSpec, FDataTableRowHandle AttackHandle);

	/**
	 * Sets the attack definition handle in an effect context.
	 * 在效果上下文中设置攻击定义句柄。
	 * @param EffectContext The effect context. 效果上下文。
	 * @param Handle The attack definition handle. 攻击定义句柄。
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|EffectContext", Meta = (DisplayName = "Set Attack Definition Handle"))
	static void EffectContextSetAttackDefinitionHandle(FGameplayEffectContextHandle EffectContext, UPARAM(meta=(RowType="/Script/SigilCombat.SigilAttackDefinition")) FDataTableRowHandle Handle);

	/**
	 * Gets the attack definition handle from an effect context.
	 * 从效果上下文中获取攻击定义句柄。
	 * @param EffectContext The effect context. 效果上下文。
	 * @return The attack definition handle. 攻击定义句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ability|EffectContext", Meta = (DisplayName = "Get Attack Definition Handle"))
	static FDataTableRowHandle EffectContextGetAttackDefinitionHandle(FGameplayEffectContextHandle EffectContext);

	/**
	 * Gets the attack definition from an effect context.
	 * 从效果上下文中获取攻击定义。
	 * @param EffectContext The effect context. 效果上下文。
	 * @return The attack definition. 攻击定义。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ability|EffectContext", Meta = (DisplayName = "Get Attack Definition"))
	static FSigilAttackDefinition EffectContextGetAttackDefinition(FGameplayEffectContextHandle EffectContext);
};