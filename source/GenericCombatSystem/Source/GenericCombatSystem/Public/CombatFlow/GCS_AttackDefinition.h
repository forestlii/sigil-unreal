// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "GGA_AbilitySystemStructLibrary.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MINOR_VERSION < 5
#include "InstancedStruct.h"
#else
#include "StructUtils/InstancedStruct.h"
#endif
#include "GCS_AttackDefinition.generated.h"

/**
 * Structure defining an attack's properties.
 * 定义攻击属性的结构。
 */
USTRUCT(BlueprintType, meta=(DisplayName="GCS Attack Definition"))
struct GENERICCOMBATSYSTEM_API FGCS_AttackDefinition : public FTableRowBase
{
	GENERATED_BODY()

	/**
	 * Tags describing the attack (e.g., Melee/Ranged, Slash/Strike).
	 * 描述攻击的标签（例如近战/远程、劈砍/打击）。
	 * @note Added as dynamic AssetTags to the gameplay effect spec.
	 * @注意 作为动态AssetTags添加到游戏效果规格。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common")
	FGameplayTagContainer AttackTags;

	/**
	 * SetByCaller tag-to-float mappings for gameplay effect specs.
	 * 用于游戏效果规格的SetByCaller标签到浮点映射。
	 * @note Usage is flexible (e.g., damage correction factors).
	 * @注意 使用灵活（例如伤害修正系数）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common", meta=(ForceInlineRow))
	TMap<FGameplayTag, float> SetByCallerMagnitudes;

	/**
	 * Gameplay effect to apply to the hit target.
	 * 应用于命中目标的游戏效果。
	 * @note Modified during attack request processing.
	 * @注意 在攻击请求处理期间修改。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> TargetEffectClass;

	/**
	 * Level of the target gameplay effect.
	 * 目标游戏效果的等级。
	 * @note If < 1, uses the ability's level.
	 * @注意 如果<1，使用能力的等级。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Effects")
	int32 TargetEffectClassLevel{1};

	/**
	 * Effect container to apply to the target.
	 * 应用于目标的效果容器。
	 * @note Used for instant targeting; ability level determines effect level.
	 * @注意 用于即时目标；能力等级决定效果等级。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Effects", meta=(ForceInlineRow))
	FGGA_GameplayEffectContainer TargetEffectContainer;

	/**
	 * Gameplay cues to trigger on the target upon hit.
	 * 命中目标时触发的游戏提示。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Cues", meta=(Categories="GameplayCue"))
	TArray<FGameplayTag> TargetGameplayCues;

	/**
	 * Knockback distance applied to the target.
	 * 应用于目标的击退距离。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReaction", meta=(Units="cm"))
	float KnockbackDistance{100};

	/**
	 * Multiplier for knockback effect.
	 * 击退效果的倍增器。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReaction", meta=(ClampMin=1))
	float KnockbackMultiplier{1};

	/**
	 * Duration of animation stall on hit (disabled if <= 0).
	 * 命中时动画停滞的持续时间（<=0时禁用）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta=(ClampMin=0, Units="s"))
	float HitStallingDuration{0};

	/**
	 * Play rate factor for hit animation.
	 * 命中动画的播放速率因子。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta=(ClampMin=0.1, ClampMax=0.9))
	float HitPlayRateFactor{0.1};

	/**
	 * User-defined settings for the attack.
	 * 攻击的用户定义设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extension", meta=(ForceInlineRow, BaseStruct = "/Script/GenericCombatSystem.GCS_UserSetting"))
	TMap<FGameplayTag, FInstancedStruct> UserSettings;
};
