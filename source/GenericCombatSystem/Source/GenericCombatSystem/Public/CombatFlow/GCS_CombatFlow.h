// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GCS_ActorOwnedObject.h"
#include "GCS_AttackResult.h"
#include "GGA_GameplayAttributeStructLibrary.h"
#include "GCS_CombatFlow.generated.h"

class UGCS_AttackResultProcessor;
class UGCS_CombatSystemComponent;

/**
 * Combat flow for processing incoming attacks.
 * 处理传入攻击的战斗流程。
 * @note Typically one instance per character type (e.g., human, quadruped, mechanical).
 * @注意 通常每种角色类型一个实例（例如人类、四足动物、机械）。
 */
UCLASS(Abstract, BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew, CollapseCategories, meta=(DisplayName="GCS Combat Flow"))
class GENERICCOMBATSYSTEM_API UGCS_CombatFlow : public UGCS_ActorOwnedObject
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	UGCS_CombatFlow();

	/**
	 * Retrieves lifetime replicated properties.
	 * 获取生命周期复制属性。
	 * @param OutLifetimeProps The lifetime properties. 生命周期属性。
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Checks if networking is supported.
	 * 检查是否支持网络。
	 * @return True if supported. 如果支持返回true。
	 */
	virtual bool IsSupportedForNetworking() const override { return true; }

	/**
	 * Gets the actor owning this combat flow.
	 * 获取拥有此战斗流程的演员。
	 * @return The owning actor. 所属演员。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Combat Flow")
	AActor* GetFlowOwner() const { return Owner; }

	/**
	 * Initializes the combat flow with an owner.
	 * 使用拥有者初始化战斗流程。
	 * @param NewOwner The owning actor. 所属演员。
	 */
	void Initialize(AActor* NewOwner);

	/**
	 * Adds dynamic tags to a gameplay effect spec.
	 * 为游戏效果规格添加动态标签。
	 * @note Requires GGA_AbilitySystemGlobals as default AbilitySystemGlobals.
	 * @注意 需要将GGA_AbilitySystemGlobals设置为默认AbilitySystemGlobals。
	 * @param Spec The gameplay effect spec. 游戏效果规格。
	 * @param AbilitySystemComponent The ability system component. 能力系统组件。
	 * @param OutDynamicTagsAppendToSpec The tags to append (output). 要附加的标签（输出）。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat Flow")
	void HandlePreGameplayEffectSpecApply(const FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer& OutDynamicTagsAppendToSpec);

	/**
	 * Handles gameplay effect execution.
	 * 处理游戏效果执行。
	 * @param Payload The effect modification data. 效果修改数据。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat Flow")
	void HandleGameplayEffectExecute(const FGGA_GameplayEffectModCallbackData& Payload);
	virtual void HandleGameplayEffectExecute_Implementation(const FGGA_GameplayEffectModCallbackData& Payload);

	/**
	 * Handles attack results across the network.
	 * 在网络上处理攻击结果。
	 * @note Default implementation calls result processors.
	 * @注意 默认实现调用结果处理器。
	 * @param Payload The attack result. 攻击结果。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCS|Combat Flow")
	void HandleAttackResult(const FGCS_AttackResult& Payload);

protected:
	/**
	 * The actor owning this combat flow.
	 * 拥有此战斗流程的演员。
	 */
	UPROPERTY()
	TObjectPtr<AActor> Owner;

	/**
	 * Reference to the owning combat system component.
	 * 所属战斗系统组件的引用。
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GCS|Combat Flow", meta=(BlueprintProtected))
	TObjectPtr<UGCS_CombatSystemComponent> CombatComponent;

	/**
	 * List of attack result processors for handling attack results.
	 * 处理攻击结果的攻击结果处理器列表。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "GCS|Combat Flow Settings", meta=(TitleProperty="EditorFriendlyName"))
	TArray<TObjectPtr<UGCS_AttackResultProcessor>> AttackResultProcessors;
};