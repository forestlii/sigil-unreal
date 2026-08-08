// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "SigilGlobalAbilitySystem.generated.h"

class USigilAbilitySystemComponent;
class UAbilitySystemComponent;
class UGameplayEffect;
class UGameplayAbility;

/**
 * Struct to track globally applied abilities.
 * 跟踪全局应用的技能的结构体。
 */
USTRUCT()
struct SIGILGAS_API FSigilGlobalAppliedAbilityList
{
	GENERATED_BODY()

	/**
	 * Map of ability system components to their ability spec handles.
	 * 技能系统组件到技能规格句柄的映射。
	 */
	UPROPERTY()
	TMap<TObjectPtr<USigilAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	/**
	 * Adds an ability to an ability system component.
	 * 将技能添加到技能系统组件。
	 * @param Ability The ability class to add. 要添加的技能类。
	 * @param ASC The ability system component. 技能系统组件。
	 */
	void AddToASC(TSubclassOf<UGameplayAbility> Ability, USigilAbilitySystemComponent* ASC);

	/**
	 * Removes an ability from an ability system component.
	 * 从技能系统组件移除技能。
	 * @param ASC The ability system component. 技能系统组件。
	 */
	void RemoveFromASC(USigilAbilitySystemComponent* ASC);

	/**
	 * Removes all abilities from all components.
	 * 从所有组件移除所有技能。
	 */
	void RemoveFromAll();
};

/**
 * Struct to track globally applied effects.
 * 跟踪全局应用的效果的结构体。
 */
USTRUCT()
struct SIGILGAS_API FSigilGlobalAppliedEffectList
{
	GENERATED_BODY()

	/**
	 * Map of ability system components to their effect handles.
	 * 技能系统组件到效果句柄的映射。
	 */
	UPROPERTY()
	TMap<TObjectPtr<USigilAbilitySystemComponent>, FActiveGameplayEffectHandle> Handles;

	/**
	 * Adds an effect to an ability system component.
	 * 将效果添加到技能系统组件。
	 * @param Effect The effect class to add. 要添加的效果类。
	 * @param ASC The ability system component. 技能系统组件。
	 */
	void AddToASC(TSubclassOf<UGameplayEffect> Effect, USigilAbilitySystemComponent* ASC);

	/**
	 * Removes an effect from an ability system component.
	 * 从技能系统组件移除效果。
	 * @param ASC The ability system component. 技能系统组件。
	 */
	void RemoveFromASC(USigilAbilitySystemComponent* ASC);

	/**
	 * Removes all effects from all components.
	 * 从所有组件移除所有效果。
	 */
	void RemoveFromAll();
};

/**
 * World subsystem for managing global abilities and effects.
 * 管理全局技能和效果的世界子系统。
 */
UCLASS()
class SIGILGAS_API USigilGlobalAbilitySystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the global ability system.
	 * 全局技能系统构造函数。
	 */
	USigilGlobalAbilitySystem();

	/**
	 * Applies an ability to all registered ability system components.
	 * 将技能应用到所有注册的技能系统组件。
	 * @param Ability The ability class to apply. 要应用的技能类。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GGA|Global")
	void ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability);

	/**
	 * Applies an effect to all registered ability system components.
	 * 将效果应用到所有注册的技能系统组件。
	 * @param Effect The effect class to apply. 要应用的效果类。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GGA|Global")
	void ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect);

	/**
	 * Removes an ability from all registered ability system components.
	 * 从所有注册的技能系统组件移除技能。
	 * @param Ability The ability class to remove. 要移除的技能类。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GGA|Global")
	void RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability);

	/**
	 * Removes an effect from all registered ability system components.
	 * 从所有注册的技能系统组件移除效果。
	 * @param Effect The effect class to remove. 要移除的效果类。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GGA|Global")
	void RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect);

	/**
	 * Registers an ability system component with the global system.
	 * 将技能系统组件注册到全局系统。
	 * @param ASC The ability system component to register. 要注册的技能系统组件。
	 */
	void RegisterASC(USigilAbilitySystemComponent* ASC);

	/**
	 * Unregisters an ability system component from the global system.
	 * 从全局系统取消注册技能系统组件。
	 * @param ASC The ability system component to unregister. 要取消注册的技能系统组件。
	 */
	void UnregisterASC(USigilAbilitySystemComponent* ASC);

private:
	/**
	 * Map of applied abilities.
	 * 已应用的技能映射。
	 */
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FSigilGlobalAppliedAbilityList> AppliedAbilities;

	/**
	 * Map of applied effects.
	 * 已应用的效果映射。
	 */
	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FSigilGlobalAppliedEffectList> AppliedEffects;

	/**
	 * List of registered ability system components.
	 * 注册的技能系统组件列表。
	 */
	UPROPERTY()
	TArray<TObjectPtr<USigilAbilitySystemComponent>> RegisteredASCs;
};