// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "SigilAbilitySystemStructLibrary.h"
#include "UObject/Interface.h"
#include "SigilEffectCauserInterface.generated.h"

class UGameplayEffect;

/**
 * Interface for objects that cause gameplay effects based on combat impact.
 * 基于战斗影响产生游戏效果的对象的接口。
 */
UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class USigilEffectCauserInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for effect causers (e.g., bullets, weapons, traps).
 * 效果触发者（例如子弹、武器、陷阱）的接口。
 */
class SIGILCOMBAT_API ISigilEffectCauserInterface
{
	GENERATED_BODY()

public:
	/**
	 * Gets a pre-existing gameplay effect spec handle.
	 * 获取预存在的游戏效果规格句柄。
	 * @param OutHandle The effect spec handle (output). 效果规格句柄（输出）。
	 * @return True if a handle is provided. 如果提供句柄返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat")
	bool GetEffectSpecHandle(FGameplayEffectSpecHandle& OutHandle);
	virtual bool GetEffectSpecHandle_Implementation(FGameplayEffectSpecHandle& OutHandle) = 0;

	/**
	 * Gets the effect container.
	 * 获取效果容器。
	 * @return The effect container. 效果容器。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat")
	FSigilGameplayEffectContainer GetEffectContainer();
	virtual FSigilGameplayEffectContainer GetEffectContainer_Implementation() const = 0;

	/**
	 * Gets the effect container level override.
	 * 获取效果容器等级覆盖。
	 * @return The level override. 等级覆盖。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat")
	int32 GetEffectContainerLevelOverride() const;
	virtual int32 GetEffectContainerLevelOverride_Implementation() const = 0;

	/**
	 * Sets the effect container spec.
	 * 设置效果容器规格。
	 * @param InEffectContainerSpec The effect container spec. 效果容器规格。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat")
	void SetEffectContainerSpec(const FSigilGameplayEffectContainerSpec& InEffectContainerSpec);
	virtual void SetEffectContainerSpec_Implementation(const FSigilGameplayEffectContainerSpec& InEffectContainerSpec) = 0;

	/**
	 * Gets the effect container spec.
	 * 获取效果容器规格。
	 * @return The effect container spec. 效果容器规格。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat")
	FSigilGameplayEffectContainerSpec GetEffectContainerSpec() const;
	virtual FSigilGameplayEffectContainerSpec GetEffectContainerSpec_Implementation() const = 0;

	/**
	 * Gets the gameplay effect class.
	 * 获取游戏效果类。
	 * @return The gameplay effect class. 游戏效果类。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat")
	TSubclassOf<UGameplayEffect> GetEffectClass() const;
	virtual TSubclassOf<UGameplayEffect> GetEffectClass_Implementation() const = 0;

	/**
	 * Gets the effect level.
	 * 获取效果等级。
	 * @return The effect level. 效果等级。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat")
	int32 GetEffectLevel() const;
	virtual int32 GetEffectLevel_Implementation() const = 0;

	/**
	 * Sets the gameplay effect spec for later use.
	 * 设置游戏效果规格以供后续使用。
	 * @param InEffectSpec The effect spec to set. 要设置的效果规格。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat")
	void SetEffectSpec(UPARAM(ref) FGameplayEffectSpecHandle& InEffectSpec);
	virtual void SetEffectSpec_Implementation(FGameplayEffectSpecHandle& InEffectSpec) = 0;
};