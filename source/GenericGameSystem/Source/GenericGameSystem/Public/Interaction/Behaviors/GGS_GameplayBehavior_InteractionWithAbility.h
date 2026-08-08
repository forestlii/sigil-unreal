// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayBehavior.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GGS_GameplayBehavior_InteractionWithAbility.generated.h"

/**
 * Gameplay behavior for ability-based interactions.
 * 基于技能的交互游戏行为。
 */
UCLASS(DisplayName="GameplayBehavior_Interaction (GGS)", NotBlueprintable)
class GENERICGAMESYSTEM_API UGGS_GameplayBehavior_InteractionWithAbility : public UGameplayBehavior
{
	GENERATED_BODY()

public:
	/**
	 * Triggers the interaction behavior.
	 * 触发交互行为。
	 * @param InAvatar The avatar actor. 化身演员。
	 * @param Config The behavior config. 行为配置。
	 * @param SmartObjectOwner The smart object owner. 智能对象拥有者。
	 * @return True if the behavior was triggered successfully, false otherwise. 如果行为成功触发返回true，否则返回false。
	 */
	virtual bool Trigger(AActor& InAvatar, const UGameplayBehaviorConfig* Config, AActor* SmartObjectOwner) override;

	/**
	 * Ends the behavior.
	 * 结束行为。
	 * @param Avatar The avatar actor. 化身演员。
	 * @param bInterrupted Whether the behavior was interrupted. 行为是否被中断。
	 */
	virtual void EndBehavior(AActor& Avatar, const bool bInterrupted) override;

	/**
	 * Checks the validity of the ability settings.
	 * 检查技能设置的有效性。
	 * @param Config The behavior config. 行为配置。
	 * @param OutAbilityClass The ability class (output). 技能类（输出）。
	 * @param OutAbilityLevel The ability level (output). 技能等级（输出）。
	 * @return True if the settings are valid, false otherwise. 如果设置有效返回true，否则返回false。
	 */
	bool CheckValidAbilitySetting(const UGameplayBehaviorConfig* Config, TSubclassOf<UGameplayAbility>& OutAbilityClass, int32& OutAbilityLevel);

	/**
	 * The ability class granted for the interaction.
	 * 为交互授予的技能类。
	 */
	UPROPERTY()
	TSubclassOf<UGameplayAbility> GrantedAbilityClass{nullptr};

	/**
	 * Handle for the granted ability spec.
	 * 授予技能规格的句柄。
	 */
	FGameplayAbilitySpecHandle AbilitySpecHandle;

	/**
	 * Indicates if the behavior was interrupted.
	 * 表示行为是否被中断。
	 */
	bool bBehaviorWasInterrupted = false;

	/**
	 * Indicates if the ability has ended.
	 * 表示技能是否已结束。
	 */
	bool bAbilityEnded = false;

	/**
	 * Indicates if the ability was cancelled.
	 * 表示技能是否被取消。
	 */
	bool bAbilityWasCancelled = false;

	/**
	 * Delegate handle for ability end notification.
	 * 技能结束通知的委托句柄。
	 */
	FDelegateHandle AbilityEndedDelegateHandle;

	/**
	 * Called when the ability ends.
	 * 技能结束时调用。
	 * @param EndedData The ability end data. 技能结束数据。
	 */
	virtual void OnAbilityEndedCallback(const FAbilityEndedData& EndedData);
};
