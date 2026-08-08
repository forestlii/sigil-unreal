// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Drops/GIS_RandomItemDropperComponent.h"
#include "GameFramework/Actor.h"
#include "GIS_InventorySystemComponent.h"
#include "GIS_ItemCollection.h"
#include "GIS_LogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_RandomItemDropperComponent)

TArray<FGIS_ItemInfo> UGIS_RandomItemDropperComponent::GetItemsToDropInternal() const
{
	TArray<FGIS_ItemInfo> Results;

	UGIS_InventorySystemComponent* Inventory = UGIS_InventorySystemComponent::FindInventorySystemComponent(GetOwner());
	if (Inventory == nullptr)
	{
		GIS_CLOG(Error, "requires inventory system component to drop items.")
		return Results;
	}
	UGIS_ItemCollection* Collection = Inventory->GetCollectionByTag(CollectionTag);
	if (Collection == nullptr)
	{
		GIS_CLOG(Error, " inventory missing collection with tag:%s'", *CollectionTag.ToString())
		return Results;
	}

	const TArray<FGIS_ItemStack>& ItemStacks = Collection->GetAllItemStacks();

	TArray<FGIS_ItemInfo> ItemInfos;
	ItemInfos.Reserve(ItemStacks.Num());
	int32 ProbabilitySum = 0;

	for (int32 i = 0; i < ItemStacks.Num(); ++i)
	{
		//加权
		ProbabilitySum += ItemStacks[i].Amount;
		ItemInfos[i] = FGIS_ItemInfo(ProbabilitySum, ItemStacks[i]);
	}

	int32 RandomAmount = FMath::RandRange(MinAmount, MaxAmount + 1);
	Results.Empty();

	for (int i = 0; i < RandomAmount; i++)
	{
		auto& selectedItemInfo = GetRandomItemInfo(ItemInfos, ProbabilitySum);
		bool foundMatch = false;
		//去重，多个栈可能指向同一个道具实例，若发现重复
		for (int j = 0; j < Results.Num(); j++)
		{
			if (Results[j].Item == selectedItemInfo.Item)
			{
				Results[j] = FGIS_ItemInfo(Results[j].Amount + 1, Results[j]);
				foundMatch = true;
				break;
			}
		}

		if (!foundMatch) { Results.Add(FGIS_ItemInfo(1, selectedItemInfo)); }
	}

	return Results;
}

const FGIS_ItemInfo& UGIS_RandomItemDropperComponent::GetRandomItemInfo(const TArray<FGIS_ItemInfo>& ItemInfos, int32 ProbabilitySum) const
{
	int32 RandomProbabilityIdx = FMath::RandRange(0, ProbabilitySum);

	int32 min = 0;
	int32 max = ItemInfos.Num() - 1;
	int32 mid = 0;

	while (min <= max)
	{
		mid = (min + max) / 2;
		if (ItemInfos[mid].Amount == RandomProbabilityIdx)
		{
			++mid;
			break;
		}

		if (RandomProbabilityIdx < ItemInfos[mid].Amount
			&& (mid == 0 || RandomProbabilityIdx > ItemInfos[mid - 1].Amount)) { break; }

		if (RandomProbabilityIdx < ItemInfos[mid].Amount) { max = mid - 1; }
		else
		{
			min = mid + 1;
		}
	}

	return ItemInfos[mid];
}
