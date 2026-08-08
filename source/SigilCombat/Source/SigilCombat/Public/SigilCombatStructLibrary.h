// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "SigilCombatStructLibrary.generated.h"

class UTargetingPreset;
class USigilCollisionTraceInstance;
class UAnimMontage;

/**
 * Structure for tagged value pairs.
 * 标记值对的结构。
 */
USTRUCT(BlueprintType)
struct SIGILCOMBAT_API FSigilTaggedValue
{
	GENERATED_BODY()

	/**
	 * The gameplay tag for the attribute.
	 * 属性的游戏标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	FGameplayTag Attribute;

	/**
	 * The value applied to the attribute.
	 * 应用于属性的值。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	float Value{0};
};

/**
 * Structure for ability actions.
 * 能力动作的结构。
 */
USTRUCT(BlueprintType)
struct SIGILCOMBAT_API FSigilAbilityAction
{
	GENERATED_BODY()

	/**
	 * The animation montage to play.
	 * 要播放的动画蒙太奇。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> Animation;

	/**
	 * The playback rate for the montage.
	 * 蒙太奇的播放速率。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float PlayRate{1.f};

	/**
	 * The starting section name for the montage.
	 * 蒙太奇的起始片段名称。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName StartSection{NAME_None};

	/**
	 * Whether to stop the montage when the ability ends.
	 * 能力结束时是否停止蒙太奇。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bStopWhenAbilityEnds{true};

	/**
	 * Scale for animation root motion translation.
	 * 动画根运动平移的缩放。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float AnimRootMotionTranslationScale{1.f};

	/**
	 * Start time for the montage in seconds.
	 * 蒙太奇的起始时间（秒）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float StartTimeSeconds{0.f};

	/**
	 * Whether to allow interruption after blend out.
	 * 是否允许在混合结束时中断。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bAllowInterruptAfterBlendOut{false};

	/**
	 * Gameplay effect for ability cost.
	 * 能力消耗的游戏效果。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffects")
	TSubclassOf<UGameplayEffect> CostGameplayEffect;

#if WITH_EDITORONLY_DATA
	/**
	 * Editor-friendly name for the ability action.
	 * 能力动作的编辑器友好名称。
	 */
	UPROPERTY(VisibleAnywhere, Category=AlwaysHidden, Meta=(EditCondition=False, EditConditionHides))
	FString EditorFriendlyName;
#endif
};

/**
 * Structure for ability actions with tag queries.
 * 带有标签查询的能力动作结构。
 */
USTRUCT(BlueprintType)
struct SIGILCOMBAT_API FSigilAbilityActionsWithQuery
{
	GENERATED_BODY()

	/**
	 * Source tag query for filtering.
	 * 用于过滤的来源标签查询。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GCS")
	FGameplayTagQuery SourceTagQuery;

	/**
	 * Target tag query for filtering.
	 * 用于过滤的目标标签查询。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GCS")
	FGameplayTagQuery TargetTagQuery;

	/**
	 * Array of ability actions.
	 * 能力动作数组。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GCS", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FSigilAbilityAction> Actions;

#if WITH_EDITORONLY_DATA
	/**
	 * Editor-friendly name for the action set.
	 * 动作集的编辑器友好名称。
	 */
	UPROPERTY(VisibleAnywhere, Category=AlwaysHidden, Meta=(EditCondition=False, EditConditionHides))
	FString EditorFriendlyName;
#endif
};

/**
 * Structure for ability action sets.
 * 能力动作集的结构。
 */
USTRUCT(BlueprintType)
struct SIGILCOMBAT_API FSigilAbilityActionSet
{
	GENERATED_BODY()

	/**
	 * The gameplay tag for the ability.
	 * 能力的游戏标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GCS")
	FGameplayTag AbilityTag;

	/**
	 * Array of ability actions.
	 * 能力动作数组。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GCS", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FSigilAbilityAction> Actions;

	/**
	 * Layered action sets for conditional selection based on tags.
	 * 基于标签条件选择的层次动作集。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GCS", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FSigilAbilityActionsWithQuery> Layered;
};

/**
 * Base structure for user settings.
 * 用户设置的基结构。
 */
USTRUCT(BlueprintType)
struct SIGILCOMBAT_API FSigilUserSetting
{
	GENERATED_BODY()
};

/**
 * User settings structure for tag-to-float mappings.
 * 标签到浮点映射的用户设置结构。
 */
USTRUCT(BlueprintType)
struct FSigilUserSetting_Attributes : public FSigilUserSetting
{
	GENERATED_BODY()

	/**
	 * Map of gameplay tags to float attributes.
	 * 游戏标签到浮点属性的映射。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UserSettings")
	TMap<FGameplayTag, float> Attributes;
};

/**
 * Structure for defining collision trace instances.
 * 定义碰撞检测实例的结构。
 */
USTRUCT(BlueprintType)
struct SIGILCOMBAT_API FSigilCollisionTraceDefinition
{
	GENERATED_BODY()

	/**
	 * Tag for the collision trace instance.
	 * 碰撞检测实例的标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	FGameplayTag TraceTag;

	/**
	 * The class of the collision trace instance.
	 * 碰撞检测实例的类。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	TSoftClassPtr<USigilCollisionTraceInstance> TraceClass;

	/**
	 * Prefix for socket names on the primitive component.
	 * 原始组件上插槽名称的前缀。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	FName SocketPrefix = NAME_None;

	/**
	 * Specific socket names; if empty, uses sockets with SocketPrefix.
	 * 特定插槽名称；如果为空，使用带有SocketPrefix的插槽。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	TArray<FName> SocketNames;

	/**
	 * Targeting preset for fetching target actors.
	 * 获取目标演员的目标预设。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	TSoftObjectPtr<UTargetingPreset> TargetingPreset;
};