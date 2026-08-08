// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MINOR_VERSION < 5
#include "InstancedStruct.h"
#else
#include "StructUtils/InstancedStruct.h"
#endif
#include "SigilGameplayEffectFunctionLibrary.generated.h"

/**
 * Blueprint function library for gameplay effect operations.
 * 用于游戏效果操作的蓝图函数库。
 */
UCLASS()
class SIGILGAS_API USigilGameplayEffectFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#pragma region EffectSpec

	/**
	 * Retrieves the magnitude of a SetByCaller modifier by tag.
	 * 通过标签获取SetByCaller修改器的大小。
	 * @param SpecHandle The gameplay effect spec handle. 游戏效果规格句柄。
	 * @param DataTag The tag to query. 要查询的标签。
	 * @param WarnIfNotFound Whether to warn if the tag is not found. 如果未找到标签是否警告。
	 * @param DefaultIfNotFound Default value if the tag is not found. 如果未找到标签的默认值。
	 * @return The magnitude value, or default if not found. 大小值，未找到时返回默认值。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayEffect")
	static float GetSetByCallerMagnitudeByTag(FGameplayEffectSpecHandle SpecHandle, FGameplayTag DataTag, bool WarnIfNotFound = false, float DefaultIfNotFound = 0.0f);

	/**
	 * Retrieves the magnitude of a SetByCaller modifier by name.
	 * 通过名称获取SetByCaller修改器的大小。
	 * @param SpecHandle The gameplay effect spec handle. 游戏效果规格句柄。
	 * @param DataName The name to query. 要查询的名称。
	 * @return The magnitude value, or zero if not found. 大小值，未找到时返回零。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayEffect")
	static float GetSetByCallerMagnitudeByName(FGameplayEffectSpecHandle SpecHandle, FName DataName);

	/**
	 * Checks if the active gameplay effect handle is valid.
	 * 检查活动游戏效果句柄是否有效。
	 * @param Handle The active gameplay effect handle. 活动游戏效果句柄。
	 * @return True if the handle is valid, false otherwise. 如果句柄有效则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayEffect")
	static bool IsActiveGameplayEffectHandleValid(FActiveGameplayEffectHandle Handle);

#pragma endregion

#pragma region Effect Context
	/**
	 * Retrieves gameplay tags from the effect context.
	 * 从效果上下文中获取游戏标签。
	 * @param EffectContext The effect context handle. 效果上下文句柄。
	 * @param ActorTagContainer Container for actor tags. 演员标签容器。
	 * @param SpecTagContainer Container for spec tags. 规格标签容器。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayEffect|Context")
	static void GetOwnedGameplayTags(FGameplayEffectContextHandle EffectContext, FGameplayTagContainer& ActorTagContainer, FGameplayTagContainer& SpecTagContainer);

	/**
	 * Sets the instigator and effect causer in the effect context.
	 * 在效果上下文中设置发起者和效果原因。
	 * @param EffectContext The effect context handle. 效果上下文句柄。
	 * @param InInstigator The instigator actor. 发起者演员。
	 * @param InEffectCauser The effect causer actor. 效果原因演员。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayEffect|Context")
	static void AddInstigator(FGameplayEffectContextHandle EffectContext, AActor* InInstigator, AActor* InEffectCauser);

	/**
	 * Sets the effect causer in the effect context.
	 * 在效果上下文中设置效果原因。
	 * @param EffectContext The effect context handle. 效果上下文句柄。
	 * @param InEffectCauser The effect causer actor. 效果原因演员。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayEffect|Context")
	static void SetEffectCauser(FGameplayEffectContextHandle EffectContext, AActor* InEffectCauser);

	/**
	 * Sets the ability in the effect context.
	 * 在效果上下文中设置技能。
	 * @param EffectContext The effect context handle. 效果上下文句柄。
	 * @param InGameplayAbility The gameplay ability to set. 要设置的游戏技能。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayEffect|Context")
	static void SetAbility(FGameplayEffectContextHandle EffectContext, const UGameplayAbility* InGameplayAbility);

	/**
	 * Retrieves the ability CDO from the effect context.
	 * 从效果上下文中获取技能CDO。
	 * @param EffectContext The effect context handle. 效果上下文句柄。
	 * @return The ability CDO. 技能CDO。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayEffect|Context")
	static const UGameplayAbility* GetAbilityCDO(FGameplayEffectContextHandle EffectContext);

	/**
	 * Retrieves the ability instance from the effect context.
	 * 从效果上下文中获取技能实例。
	 * @param EffectContext The effect context handle. 效果上下文句柄。
	 * @return The ability instance. 技能实例。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayEffect|Context")
	static const UGameplayAbility* GetAbilityInstance(FGameplayEffectContextHandle EffectContext);

	/**
	 * Retrieves the ability level from the effect context.
	 * 从效果上下文中获取技能等级。
	 * @param EffectContext The effect context handle. 效果上下文句柄。
	 * @return The ability level. 技能等级。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayEffect|Context")
	static int32 GetAbilityLevel(FGameplayEffectContextHandle EffectContext);

	/**
	 * Retrieves the instigator's ability system component.
	 * 获取发起者的技能系统组件。
	 * @param EffectContext The effect context handle. 效果上下文句柄。
	 * @return The instigator's ability system component. 发起者的技能系统组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayEffect|Context")
	static UAbilitySystemComponent* GetInstigatorAbilitySystemComponent(FGameplayEffectContextHandle EffectContext);

	/**
	 * Retrieves the original instigator's ability system component.
	 * 获取原始发起者的技能系统组件。
	 * @param EffectContext The effect context handle. 效果上下文句柄。
	 * @return The original instigator's ability system component. 原始发起者的技能系统组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayEffect|Context")
	static UAbilitySystemComponent* GetOriginalInstigatorAbilitySystemComponent(FGameplayEffectContextHandle EffectContext);

	/**
	 * Sets the source object in the effect context.
	 * 在效果上下文中设置源对象。
	 * @param EffectContext The effect context handle. 效果上下文句柄。
	 * @param NewSourceObject The source object to set. 要设置的源对象。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayEffect|Context")
	static void AddSourceObject(FGameplayEffectContextHandle EffectContext, const UObject* NewSourceObject);

	/**
	 * Checks if the effect context has an origin.
	 * 检查效果上下文是否具有起源。
	 * @param EffectContext The effect context handle. 效果上下文句柄。
	 * @return True if the context has an origin, false otherwise. 如果上下文有起源则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayEffect|Context")
	static bool HasOrigin(FGameplayEffectContextHandle EffectContext);

#pragma endregion
};
