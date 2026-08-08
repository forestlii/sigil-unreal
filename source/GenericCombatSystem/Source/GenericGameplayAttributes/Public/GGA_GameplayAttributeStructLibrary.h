// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "GGA_GameplayAttributeStructLibrary.generated.h"

USTRUCT(BlueprintType)
struct GENERICGAMEPLAYATTRIBUTES_API FGGA_ModifiedAttribute
{
	GENERATED_BODY()

	/** The attribute that has been modified */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	FGameplayAttribute Attribute;

	/** Total magnitude applied to that attribute */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	float TotalMagnitude{0};
};


/**
 * Information about PostGameplayEffectExecute event with related objects cached.
 * 封装后的关于PostGameplayEffectExecute的回调数据，并提取相关对象以方便调用。
 */
USTRUCT(BlueprintType)
struct GENERICGAMEPLAYATTRIBUTES_API FGGA_GameplayEffectModCallbackData
{
	GENERATED_BODY()

	/**
	 * The owner AttributeSet from which the event was invoked
	 * 变化的属性所属的AttributeSet.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;

	/**
	 * Evaluated data about this change.
	 * 关于这次变化的结果
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	FGameplayModifierEvaluatedData EvaluatedData;

	/**
	 * Any modified attributes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	TArray<FGGA_ModifiedAttribute> ModifiedAttributes;

	/**
	 * Map of set by caller magnitudes
	 * 执行过程中设置的临时值。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	TMap<FName, float> SetByCallerNameMagnitudes;

	/**
	 * Map of set by caller magnitudes
	 * 执行过程中设置的临时值。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	TMap<FGameplayTag, float> SetByCallerTagMagnitudes;

	/**
	 * The context of The effect spec that the mod came from
	 * 触发这次修改的效果实例的上下文。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	FGameplayEffectContextHandle ContextHandle;

	/**
	 * The instigator actor within context.
	 * 上下文中的Instigator Actor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	TWeakObjectPtr<AActor> InstigatorActor = nullptr;

	/**
	 * The target actor within context.
	 * 上下文中的目标Actor。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	/**
	 * Target we intend to apply to
	 * 该效果的施加对象。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	TObjectPtr<UAbilitySystemComponent> TargetAsc = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	FGameplayTagContainer AggregatedSourceTags = FGameplayTagContainer::EmptyContainer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	FGameplayTagContainer AggregatedTargetTags = FGameplayTagContainer::EmptyContainer;
};
