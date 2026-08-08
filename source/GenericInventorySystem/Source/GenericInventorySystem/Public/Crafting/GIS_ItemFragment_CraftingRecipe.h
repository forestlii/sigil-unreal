// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GIS_CoreStructLibray.h"
#include "GIS_CurrencyEntry.h"
#include "GIS_ItemFragment.h"
#include "GIS_ItemFragment_CraftingRecipe.generated.h"

/**
 * Item fragment defining a crafting recipe for producing items.
 * 定义用于生产道具的合成配方的道具片段。
 * @details Specifies input items, currencies, and output items for crafting.
 * @细节 指定用于合成的输入道具、货币和输出道具。
 */
UCLASS(DisplayName="Crafting Recipe Settings", Category="BuiltIn")
class GENERICINVENTORYSYSTEM_API UGIS_ItemFragment_CraftingRecipe : public UGIS_ItemFragment
{
	GENERATED_BODY()

public:
	/**
	 * List of required items and their quantities for crafting.
	 * 合成所需的道具及其数量列表。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Crafting", meta=(TitleProperty="{Definition}->{Amount}"))
	TArray<FGIS_ItemDefinitionAmount> InputItems;

	/**
	 * List of required currencies and their amounts for crafting.
	 * 合成所需的货币及其数量列表。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ingredients", meta=(TitleProperty="{Tag}->{Amount}"))
	TArray<FGIS_CurrencyEntry> InputCurrencies;

	/**
	 * List of items produced by the crafting recipe.
	 * 合成配方产出的道具列表。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Crafting", meta=(TitleProperty="{DefinitionTag}->{Amount}"))
	TArray<FGIS_ItemDefinitionAmount> OutputItems;
};
