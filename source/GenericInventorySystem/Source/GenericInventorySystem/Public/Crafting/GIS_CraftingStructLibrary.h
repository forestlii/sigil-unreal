// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "GIS_CoreStructLibray.h"
#include "GIS_CurrencyEntry.h"
#include "Items/GIS_ItemInfo.h"
#include "UObject/Object.h"
#include "GIS_CraftingStructLibrary.generated.h"

/**
 * Struct for querying item ingredients required for crafting.
 * 用于查询合成所需道具材料的结构体。
 */
USTRUCT(BlueprintType)
struct FGIS_ItemIngredient
{
	GENERATED_USTRUCT_BODY()

	FGIS_ItemIngredient()
		: NeedAmounts(0)
	{
	}

	FGIS_ItemIngredient(int32 InNeedAmounts, const TArray<FGIS_ItemInfo>& InHoldItemAmounts)
		: NeedAmounts(InNeedAmounts)
		  , HoldItemAmounts(InHoldItemAmounts)
	{
	}

	/**
	 * Definition of the item required for crafting.
	 * 合成所需道具的定义。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinition;

	/**
	 * Required amount of the item for crafting.
	 * 合成所需道具的数量。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	int32 NeedAmounts;

	/**
	 * List of held items and their amounts.
	 * 持有道具及其数量的列表。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	TArray<FGIS_ItemInfo> HoldItemAmounts;

	/**
	 * Calculates the total amount of held items.
	 * 计算持有道具的总量。
	 * @return The total amount of held items. 持有道具的总量。
	 */
	FORCEINLINE int32 GetHoldAmount() const
	{
		int32 Count = 0;
		for (const auto& ItemInfo : HoldItemAmounts)
		{
			Count += ItemInfo.Amount;
		}
		return Count;
	}
};

/**
 * Struct for querying currency ingredients required for crafting.
 * 用于查询合成所需货币材料的结构体。
 */
USTRUCT(BlueprintType)
struct FGIS_CurrencyIngredient
{
	GENERATED_USTRUCT_BODY()

	FGIS_CurrencyIngredient()
		: NeedAmounts(0)
	{
	}

	FGIS_CurrencyIngredient(TObjectPtr<const UGIS_CurrencyDefinition> InCurrencyDefinition, int32 InNeedAmounts, FGIS_CurrencyEntry InHoldAmounts)
		: CurrencyDefinition(InCurrencyDefinition)
		  , NeedAmounts(InNeedAmounts)
		  , HoldCurrencyInfo(InHoldAmounts)
	{
	}

	/**
	 * Definition of the currency required for crafting.
	 * 合成所需货币的定义。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	TObjectPtr<const UGIS_CurrencyDefinition> CurrencyDefinition;

	/**
	 * Required amount of the currency for crafting.
	 * 合成所需货币的数量。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	int32 NeedAmounts;

	/**
	 * Information about the held currency.
	 * 持有货币的信息。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	FGIS_CurrencyEntry HoldCurrencyInfo;
};

/**
 * Struct defining the cost data for crafting or enhancing items.
 * 定义合成或强化道具的消耗数据的结构体。
 */
USTRUCT(BlueprintType)
struct FGIS_CraftItemCostData
{
	GENERATED_USTRUCT_BODY()

	FGIS_CraftItemCostData() : bCostItem(false), bCostCurrency(true), bCostTime(false), Days(1)
	{
	};

	/**
	 * Determines if items are consumed during crafting.
	 * 确定合成时是否消耗道具。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
	bool bCostItem;

	/**
	 * List of items consumed during crafting.
	 * 合成时消耗的道具列表。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bCostItem"), Category="Crafting", meta=(TitleProperty="Definition"))
	TArray<FGIS_ItemDefinitionAmount> CostItems;

	/**
	 * Determines if currencies are consumed during crafting.
	 * 确定合成时是否消耗货币。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
	bool bCostCurrency;

	/**
	 * List of currencies consumed during crafting.
	 * 合成时消耗的货币列表。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bCostCurrency"), Category="Crafting")
	TArray<FGIS_CurrencyEntry> CostCurrencies;

	/**
	 * Determines if time is consumed during crafting.
	 * 确定合成时是否消耗时间。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
	bool bCostTime;

	/**
	 * Time consumed for crafting, in days.
	 * 合成消耗的时间（以天为单位）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bCostTime"), Category="Crafting")
	float Days;
};

/**
 * Data table struct for item crafting recipes.
 * 道具合成配方的数据表结构体。
 */
USTRUCT(BlueprintType)
struct FGIS_CraftingItemData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	/**
	 * Gameplay tag defining the crafting item.
	 * 定义合成道具的游戏标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	FGameplayTag CraftingItemDefinition;

	/**
	 * Definition of the crafting item.
	 * 合成道具的定义。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	TSoftObjectPtr<const UGIS_ItemDefinition> CraftingDefinition;

	/**
	 * Type of crafting operation.
	 * 合成操作的类型。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	FGameplayTag CraftingItemType;

	/**
	 * Cost data for the crafting operation.
	 * 合成操作的消耗数据。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	FGIS_CraftItemCostData CraftItemCost;

	/**
	 * Definition of the output item produced by crafting.
	 * 合成产出的道具定义。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GIS")
	TSoftObjectPtr<const UGIS_ItemDefinition> OutputItemDefinition;
};

/**
 * Data table struct for item enhancement data.
 * 道具强化数据的数据表结构体。
 */
USTRUCT(BlueprintType)
struct FGIS_ItemEnhancedData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	FGIS_ItemEnhancedData() : EnhancedLevel(1), bDestroy(false), Downgrading(false)
	{
	};

	/**
	 * Set of item types that can use this enhancement data.
	 * 可以使用此强化数据的道具类型集合。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	FGameplayTagContainer ItemTypeSet;

	/**
	 * Tag query for items eligible for this enhancement.
	 * 适用于此强化的道具标签查询。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	FGameplayTagQuery ItemTagQuery;

	/**
	 * Level of enhancement applied to the item.
	 * 应用于道具的强化等级。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	int32 EnhancedLevel;

	/**
	 * Prefix added to the item name after enhancement.
	 * 强化后添加到道具名称的前缀。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	FName EnhancedNamePrefix;

	/**
	 * Cost data for the enhancement operation.
	 * 强化操作的消耗数据。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	FGIS_CraftItemCostData EnhancedItemCost;

	/**
	 * Constant attribute bonuses applied by the enhancement.
	 * 强化应用的常量属性加成。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GIS")
	TMap<FGameplayTag, float> AdditionAttributes;

	/**
	 * Multiplier attribute bonuses applied by the enhancement.
	 * 强化应用的系数属性加成。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GIS")
	TMap<FGameplayTag, float> MultiplierAttributes;

	/**
	 * Success rate of the enhancement operation.
	 * 强化操作的成功率。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GIS", meta=(ClampMin = 0.f, ClampMax = 1.f))
	float SuccessRate = 1.f;

	/**
	 * Determines if the item is destroyed on enhancement failure (deprecated).
	 * 确定强化失败时道具是否销毁（已弃用）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GIS", meta=(DeprecatedProperty))
	bool bDestroy;

	/**
	 * Level reduction on enhancement failure (deprecated).
	 * 强化失败时的等级降低（已弃用）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GIS", meta=(EditCondition="!bDestroy", DeprecatedProperty))
	int32 Downgrading;
};
