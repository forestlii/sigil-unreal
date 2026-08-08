// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "GGA_AbilitySystemStructLibrary.h"
#include "GGA_AbilitySystemGlobals.generated.h"

class IGGA_AbilitySystemGlobalsEventReceiver;

/**
 * Extended ability system globals for custom functionality.
 * 扩展的技能系统全局类，提供自定义功能。
 */
UCLASS()
class GENERICGAMEPLAYABILITIES_API UGGA_AbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	/**
	 * Processes gameplay effect specs before application.
	 * 在应用游戏效果规格前进行处理。
	 * @param Spec The gameplay effect spec. 游戏效果规格。
	 * @param AbilitySystemComponent The ability system component. 技能系统组件。
	 */
	virtual void GlobalPreGameplayEffectSpecApply(FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent) override;

	/**
	 * Retrieves the ability system globals instance.
	 * 获取技能系统全局实例。
	 * @return The ability system globals instance. 技能系统全局实例。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|Globals")
	static const UAbilitySystemGlobals* GetAbilitySystemGlobals();

	/**
	 * Retrieves a typed ability system globals instance.
	 * 获取特定类型的技能系统全局实例。
	 * @param DesiredClass The desired class type. 所需的类类型。
	 * @return The typed ability system globals instance. 特定类型的技能系统全局实例。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|Globals", meta=(DynamicOutputParam=ReturnValue, DeterminesOutputType=DesiredClass))
	static const UAbilitySystemGlobals* GetTypedAbilitySystemGloabls(TSubclassOf<UAbilitySystemGlobals> DesiredClass);

	/**
	 * Registers an event receiver for global events.
	 * 为全局事件注册事件接收器。
	 * @param NewReceiver The event receiver to register. 要注册的事件接收器。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Globals", meta=(DefaultToSelf="NewReceiver"))
	static void RegisterEventReceiver(TScriptInterface<IGGA_AbilitySystemGlobalsEventReceiver> NewReceiver);

	/**
	 * Unregisters an event receiver from global events.
	 * 从全局事件取消注册事件接收器。
	 * @param NewReceiver The event receiver to unregister. 要取消注册的事件接收器。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Globals", meta=(DefaultToSelf="NewReceiver"))
	static void UnregisterEventReceiver(TScriptInterface<IGGA_AbilitySystemGlobalsEventReceiver> NewReceiver);

	/**
	 * Retrieves all currently loaded attribute default curve tables.
	 * 获取当前加载的所有属性默认曲线表。
	 * @return Array of curve tables. 曲线表数组。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|Globals")
	TArray<UCurveTable*> GetAttributeDefaultsTables() const;

	/**
	 * Initializes attribute set defaults for an ability system component.
	 * 为技能系统组件初始化属性集默认值。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param GroupName The attribute group name. 属性组名称。
	 * @param Level The level to initialize. 初始化等级。
	 * @param bInitialInit Whether this is the initial initialization. 是否为初始初始化。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "GGA|Globals")
	void InitAttributeSetDefaults(UAbilitySystemComponent* AbilitySystem, const FGGA_AttributeGroupName& GroupName, int32 Level = 1, bool bInitialInit = false) const;

	/**
	 * Applies a single attribute default to an ability system component.
	 * 为技能系统组件应用单个属性默认值。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param InAttribute The attribute to apply. 要应用的属性。
	 * @param GroupName The attribute group name. 属性组名称。
	 * @param Level The level to apply. 应用等级。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "GGA|Globals")
	void ApplyAttributeDefault(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute& InAttribute, const FGGA_AttributeGroupName& GroupName, int32 Level) const;

private:
	/**
	 * List of registered event receivers.
	 * 注册的事件接收器列表。
	 */
	UPROPERTY()
	TArray<TScriptInterface<IGGA_AbilitySystemGlobalsEventReceiver>> Receivers;
};

/**
 * Interface for receiving global ability system events.
 * 接收全局技能系统事件的接口。
 */
UINTERFACE()
class GENERICGAMEPLAYABILITIES_API UGGA_AbilitySystemGlobalsEventReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implementation class for global ability system event receiver.
 * 全局技能系统事件接收器的实现类。
 */
class GENERICGAMEPLAYABILITIES_API IGGA_AbilitySystemGlobalsEventReceiver
{
	GENERATED_BODY()

public:
	/**
	 * Handles global pre-gameplay effect spec application.
	 * 处理全局游戏效果规格应用前的事件。
	 * @param Spec The gameplay effect spec. 游戏效果规格。
	 * @param AbilitySystemComponent The ability system component. 技能系统组件。
	 */
	virtual void ReceiveGlobalPreGameplayEffectSpecApply(FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent);

protected:
	/**
	 * Virtual function for handling pre-gameplay effect spec application.
	 * 处理游戏效果规格应用前的虚函数。
	 * @param Spec The gameplay effect spec. 游戏效果规格。
	 * @param AbilitySystemComponent The ability system component. 技能系统组件。
	 */
	virtual void OnGlobalPreGameplayEffectSpecApply(FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent) = 0;

	/**
	 * Blueprint event for handling pre-gameplay effect spec application.
	 * 处理游戏效果规格应用前的蓝图事件。
	 * @param Spec The gameplay effect spec. 游戏效果规格。
	 * @param AbilitySystemComponent The ability system component. 技能系统组件。
	 * @param OutDynamicTagsAppendToSpec Tags to append to the spec (output). 要附加到规格的标签（输出）。
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="GGA|Global", meta=(DisplayName="On Global Pre Gameplay Effect Spec Apply"))
	void OnGlobalPreGameplayEffectSpecApply_Bp(const FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer& OutDynamicTagsAppendToSpec);
};
