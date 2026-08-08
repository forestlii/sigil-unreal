// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GGA_AbilitySystemStructLibrary.generated.h"

class UTargetingPreset;
class UGameplayEffect;

/**
 * Struct for defining an attribute group name.
 * 定义属性组名称的结构体。
 */
USTRUCT(BlueprintType)
struct GENERICGAMEPLAYABILITIES_API FGGA_AttributeGroupName
{
	GENERATED_BODY()

public:
	/**
	 * Main name of the attribute group.
	 * 属性组的主名称。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGA")
	FName MainName{NAME_None};
	
	/**
	 * Sub-name of the attribute group.
	 * 属性组的子名称。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGA")
	FName SubName{NAME_None};

	/**
	 * Checks if the group name is valid.
	 * 检查组名称是否有效。
	 * @return True if valid, false otherwise. 如果有效则返回true，否则返回false。
	 */
	bool IsValid() const { return !MainName.IsNone(); }

	/**
	 * Retrieves the combined group name.
	 * 获取组合的组名称。
	 * @return The combined name. 组合名称。
	 */
	FName GetName() const
	{
		FName Ref = MainName;
		if (!SubName.IsNone())
		{
			Ref = FName(*FString::Printf(TEXT("%s.%s"), *Ref.ToString(), *SubName.ToString()));
		}
		return Ref;
	}
};

/**
 * Struct for tracking gameplay tag counts.
 * 跟踪游戏标签计数的结构体。
 */
USTRUCT(BlueprintType)
struct GENERICGAMEPLAYABILITIES_API FGGA_GameplayTagCount
{
	GENERATED_BODY()

	/**
	 * The gameplay tag.
	 * 游戏标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	FGameplayTag Tag;

	/**
	 * The count of the tag.
	 * 标签的计数。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	int32 Count{0};
};

/**
 * Struct for storing an array of gameplay tags.
 * 存储游戏标签数组的结构体。
 */
USTRUCT(BlueprintType)
struct GENERICGAMEPLAYABILITIES_API FGGA_GameplayTagArray
{
	GENERATED_BODY()

	/**
	 * Array of gameplay tags.
	 * 游戏标签数组。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	TArray<FGameplayTag> GameplayTags;

	/**
	 * Converts the array to a gameplay tag container.
	 * 将数组转换为游戏标签容器。
	 * @return The gameplay tag container. 游戏标签容器。
	 */
	FORCEINLINE FGameplayTagContainer GetGameplayTagContainer() const
	{
		return FGameplayTagContainer::CreateFromArray(GameplayTags);
	}

	/**
	 * Creates a tag array from a gameplay tag container.
	 * 从游戏标签容器创建标签数组。
	 * @param Container The gameplay tag container. 游戏标签容器。
	 * @return The created tag array. 创建的标签数组。
	 */
	FORCEINLINE static FGGA_GameplayTagArray CreateFromContainer(const FGameplayTagContainer& Container)
	{
		FGGA_GameplayTagArray TagArray;
		Container.GetGameplayTagArray(TagArray.GameplayTags);
		return TagArray;
	}
};

/**
 * Struct for storing an array of gameplay effects.
 * 存储游戏效果数组的结构体。
 */
USTRUCT(BlueprintType)
struct GENERICGAMEPLAYABILITIES_API FGGA_GameplayEffectArray
{
	GENERATED_BODY()

	/**
	 * Array of gameplay effect classes.
	 * 游戏效果类数组。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffects;
};

/**
 * Struct defining a gameplay effect container with targeting info.
 * 定义带有目标信息的游戏效果容器的结构体。
 */
USTRUCT(BlueprintType)
struct GENERICGAMEPLAYABILITIES_API FGGA_GameplayEffectContainer
{
	GENERATED_BODY()

public:
	FGGA_GameplayEffectContainer() {}

	/**
	 * Targeting preset for the effect container.
	 * 效果容器的目标预设。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GGA")
	TObjectPtr<UTargetingPreset> TargetingPreset;

	/**
	 * List of gameplay effect classes to apply.
	 * 要应用的游戏效果类列表。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GGA")
	TArray<TSubclassOf<UGameplayEffect>> TargetGameplayEffectClasses;
};

/**
 * Processed gameplay effect container spec for application.
 * 用于应用的处理过的游戏效果容器规格。
 */
USTRUCT(BlueprintType)
struct GENERICGAMEPLAYABILITIES_API FGGA_GameplayEffectContainerSpec
{
	GENERATED_BODY()

public:
	FGGA_GameplayEffectContainerSpec() {}

	/**
	 * Computed target data for the effect.
	 * 效果的计算目标数据。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	FGameplayAbilityTargetDataHandle TargetData;

	/**
	 * List of gameplay effect specs to apply.
	 * 要应用的游戏效果规格列表。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GGA")
	TArray<FGameplayEffectSpecHandle> TargetGameplayEffectSpecs;

	/**
	 * Checks if the spec has valid effects.
	 * 检查规格是否具有有效效果。
	 * @return True if valid effects exist, false otherwise. 如果存在有效效果则返回true，否则返回false。
	 */
	bool HasValidEffects() const;

	/**
	 * Checks if the spec has valid targets.
	 * 检查规格是否具有有效目标。
	 * @return True if valid targets exist, false otherwise. 如果存在有效目标则返回true，否则返回false。
	 */
	bool HasValidTargets() const;

	/**
	 * Adds targets to the target data.
	 * 将目标添加到目标数据。
	 * @param HitResults Array of hit results. 命中结果数组。
	 * @param TargetActors Array of target actors. 目标演员数组。
	 */
	void AddTargets(const TArray<FHitResult>& HitResults, const TArray<AActor*>& TargetActors);
};