// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_InventoryFunctionLibrary.h"

#include "GIS_ItemCollection.h"
#include "Items/GIS_ItemInstance.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_InventoryFunctionLibrary)

TArray<FGIS_ItemDefinitionAmount> UGIS_InventoryFunctionLibrary::MultiplyItemAmounts(const TArray<FGIS_ItemDefinitionAmount>& ItemAmounts, int32 Multiplier)
{
	TArray<FGIS_ItemDefinitionAmount> Results;
	for (int32 i = 0; i < ItemAmounts.Num(); i++)
	{
		Results.Add(FGIS_ItemDefinitionAmount(ItemAmounts[i].Definition, ItemAmounts[i].Amount * Multiplier));
	}
	return Results;
}

TArray<FGIS_CurrencyEntry> UGIS_InventoryFunctionLibrary::MultiplyCurrencies(const TArray<FGIS_CurrencyEntry>& Currencies, float Multiplier)
{
	TArray<FGIS_CurrencyEntry> Results;
	for (int32 i = 0; i < Currencies.Num(); i++)
	{
		Results.Add(FGIS_CurrencyEntry(Currencies[i].Definition, Currencies[i].Amount * Multiplier));
	}
	return Results;
}

TArray<FGIS_ItemInfo> UGIS_InventoryFunctionLibrary::FilterItemInfosByTagQuery(const TArray<FGIS_ItemInfo>& ItemInfos, const FGameplayTagQuery& Query)
{
	return ItemInfos.FilterByPredicate([&](const FGIS_ItemInfo& ItemInfo)
	{
		return ItemInfo.Item != nullptr && ItemInfo.Item->GetItemTags().MatchesQuery(Query);
	});
}


TArray<FGIS_ItemStack> UGIS_InventoryFunctionLibrary::FilterItemStacksByTagQuery(const TArray<FGIS_ItemStack>& ItemStacks, const FGameplayTagQuery& TagQuery)
{
	return ItemStacks.FilterByPredicate([TagQuery](const FGIS_ItemStack& Stack)
	{
		return TagQuery.Matches(Stack.Item->GetItemTags());
	});
}

TArray<FGIS_ItemStack> UGIS_InventoryFunctionLibrary::FilterItemStacksByDefinition(const TArray<FGIS_ItemStack>& ItemStacks, const UGIS_ItemDefinition* Definition)
{
	return ItemStacks.FilterByPredicate([Definition](const FGIS_ItemStack& Stack)
	{
		return Stack.Item->GetDefinition() == Definition;
	});
}

TArray<FGIS_ItemStack> UGIS_InventoryFunctionLibrary::FilterItemStacksByCollectionTags(const TArray<FGIS_ItemStack>& ItemStacks, const FGameplayTagContainer& CollectionTags)
{
	return ItemStacks.FilterByPredicate([CollectionTags](const FGIS_ItemStack& Stack)
	{
		return Stack.Collection->GetCollectionTag().MatchesAnyExact(CollectionTags);
	});
}
