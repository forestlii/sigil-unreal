// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SigilGameplayAbilityFunctionLibrary.generated.h"

class UGameplayAbility;

/**
 * Blueprint function library for gameplay ability operations.
 * 用于游戏技能操作的蓝图函数库。
 */
UCLASS()
class SIGILGAS_API USigilGameplayAbilityFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Checks if an ability spec handle is valid.
	 * 检查技能规格句柄是否有效。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @return True if the handle is valid, false otherwise. 如果句柄有效则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAbility")
	static bool IsAbilitySpecHandleValid(FGameplayAbilitySpecHandle Handle);

	/**
	 * Retrieves the default object for an ability class.
	 * 获取技能类的默认对象。
	 * @param AbilityClass The ability class. 技能类。
	 * @return The default object for the ability class. 技能类的默认对象。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAbility")
	static const UGameplayAbility* GetAbilityCDOFromClass(TSubclassOf<UGameplayAbility> AbilityClass);

	/**
	 * Retrieves the current ability spec handle.
	 * 获取当前技能规格句柄。
	 * @param Ability The gameplay ability. 游戏技能。
	 * @return The ability spec handle. 技能规格句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category= "GGA|GameplayAbility", meta = (DefaultToSelf="Ability"))
	static FGameplayAbilitySpecHandle GetCurrentAbilitySpecHandle(const UGameplayAbility* Ability);

	/**
	 * Checks if an ability is currently active.
	 * 检查技能是否当前激活。
	 * @param Ability The gameplay ability. 游戏技能。
	 * @return True if the ability is active, false otherwise. 如果技能激活则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category= "GGA|GameplayAbility", meta = (DefaultToSelf="Ability"))
	static bool IsAbilityActive(const UGameplayAbility* Ability);

	/**
	 * Retrieves the replication policy for an ability.
	 * 获取技能的复制策略。
	 * @param Ability The gameplay ability. 游戏技能。
	 * @return The replication policy. 复制策略。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category= "GGA|GameplayAbility", meta = (DefaultToSelf="Ability"))
	static EGameplayAbilityReplicationPolicy::Type GetReplicationPolicy(const UGameplayAbility* Ability);

	/**
	 * Retrieves the instancing policy for an ability.
	 * 获取技能的实例化策略。
	 * @param Ability The gameplay ability. 游戏技能。
	 * @return The instancing policy. 实例化策略。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category= "GGA|GameplayAbility", meta = (DefaultToSelf="Ability"))
	static EGameplayAbilityInstancingPolicy::Type GetInstancingPolicy(const UGameplayAbility* Ability);

	/**
	 * Retrieves the tags associated with an ability.
	 * 获取与技能关联的标签。
	 * @param Ability The gameplay ability. 游戏技能。
	 * @return The gameplay tag container. 游戏标签容器。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAbility", meta = (DefaultToSelf="Ability"))
	static FGameplayTagContainer GetAbilityTags(const UGameplayAbility* Ability);

	/**
	 * Checks if the ability is running on a predicting client.
	 * 检查技能是否在预测客户端上运行。
	 * @param Ability The gameplay ability. 游戏技能。
	 * @return True if running on a predicting client, false otherwise. 如果在预测客户端上运行则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAbility", meta = (DefaultToSelf="Ability"))
	static bool IsPredictingClient(const UGameplayAbility* Ability);

	/**
	 * Checks if the ability is for a remote client.
	 * 检查技能是否用于远程客户端。
	 * @param Ability The gameplay ability. 游戏技能。
	 * @return True if for a remote client, false otherwise. 如果用于远程客户端则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAbility", meta = (DefaultToSelf="Ability"))
	static bool IsForRemoteClient(const UGameplayAbility* Ability);

	/**
	 * Checks if the ability has authority or a valid prediction key.
	 * 检查技能是否具有权限或有效预测键。
	 * @param Ability The gameplay ability. 游戏技能。
	 * @return True if it has authority or a valid prediction key, false otherwise. 如果具有权限或有效预测键则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAbility", meta = (DefaultToSelf="Ability"))
	static bool HasAuthorityOrPredictionKey(const UGameplayAbility* Ability);
};