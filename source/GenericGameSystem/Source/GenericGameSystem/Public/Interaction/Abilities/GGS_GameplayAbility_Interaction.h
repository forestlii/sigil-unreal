// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SmartObjectRuntime.h"
#include "Abilities/GameplayAbility.h"
#include "GGS_GameplayAbility_Interaction.generated.h"

class UGGS_InteractionSystemComponent;
class USmartObjectComponent;

/**
 * Core gameplay ability for handling interactions.
 * 处理交互的核心游戏技能。
 */
UCLASS(BlueprintType, Blueprintable)
class GENERICGAMESYSTEM_API UGGS_GameplayAbility_Interaction : public UGameplayAbility
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the interaction gameplay ability.
	 * 交互游戏技能构造函数。
	 */
	UGGS_GameplayAbility_Interaction();

	/**
	 * Activates the interaction ability.
	 * 激活交互技能。
	 * @param Handle The ability specification handle. 技能规格句柄。
	 * @param ActorInfo Information about the actor using the ability. 使用技能的演员信息。
	 * @param ActivationInfo Information about the ability activation. 技能激活信息。
	 * @param TriggerEventData Optional event data triggering the ability. 触发技能的可选事件数据。
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

	/**
	 * Ends the interaction ability.
	 * 结束交互技能。
	 * @param Handle The ability specification handle. 技能规格句柄。
	 * @param ActorInfo Information about the actor using the ability. 使用技能的演员信息。
	 * @param ActivationInfo Information about the ability activation. 技能激活信息。
	 * @param bReplicateEndAbility Whether to replicate the end ability call. 是否同步结束技能调用。
	 * @param bWasCancelled Whether the ability was cancelled. 技能是否被取消。
	 */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

protected:
	/**
	 * Attempts to claim an interaction with a smart object.
	 * 尝试认领与智能对象的交互。
	 * @param Index The interaction option index. 交互选项索引。
	 * @param ClaimedHandle The claimed smart object handle (output). 认领的智能对象句柄（输出）。
	 * @return True if the interaction was claimed successfully, false otherwise. 如果交互成功认领返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GCS|Interaction", meta=(ExpandBoolAsExecs=ReturnValue))
	bool TryClaimInteraction(int32 Index, FSmartObjectClaimHandle& ClaimedHandle);

	/**
	 * Called when the interactable actor changes.
	 * 可交互演员变更时调用。
	 * @param OldActor The previous interactable actor. 之前的可交互演员。
	 * @param NewActor The new interactable actor. 新的可交互演员。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GCS|Interaction")
	void OnInteractActorChanged(AActor* OldActor, AActor* NewActor);

	/**
	 * Reference to the interaction system component.
	 * 交互系统组件的引用。
	 */
	UPROPERTY(BlueprintReadOnly, Category="GCS|Interaction")
	TObjectPtr<UGGS_InteractionSystemComponent> InteractionSystem{nullptr};

#if WITH_EDITORONLY_DATA
	/**
	 * Validates data in the editor.
	 * 在编辑器中验证数据。
	 * @param Context The data validation context. 数据验证上下文。
	 * @return The validation result. 验证结果。
	 */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};