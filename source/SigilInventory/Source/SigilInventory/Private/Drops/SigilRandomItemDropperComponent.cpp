// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Drops/SigilRandomItemDropperComponent.h"
#include "GameFramework/Actor.h"
#include "SigilInventorySystemComponent.h"
#include "SigilItemCollection.h"
#include "SigilInventoryLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilRandomItemDropperComponent)

TArray<FSigilItemInfo> USigilRandomItemDropperComponent::GetItemsToDropInternal() const
{
	TArray<FSigilItemInfo> Results;

	USigilInventorySystemComponent* Inventory = USigilInventorySystemComponent::FindInventorySystemComponent(GetOwner());
	if (Inventory == nullptr)
	{
		SIGIL_INVENTORY_CLOG(Error, "requires inventory system component to drop items.")
		return Results;
	}
	USigilItemCollection* Collection = Inventory->GetCollectionByTag(CollectionTag);
	if (Collection == nullptr)
	{
		SIGIL_INVENTORY_CLOG(Error, " inventory missing collection with tag:%s'", *CollectionTag.ToString())
		return Results;
	}

	const TArray<FSigilItemStack>& ItemStacks = Collection->GetAllItemStacks();

	TArray<FSigilItemInfo> ItemInfos;
	ItemInfos.Reserve(ItemStacks.Num());
	int32 ProbabilitySum = 0;

	for (int32 i = 0; i < ItemStacks.Num(); ++i)
	{
		//加权
		ProbabilitySum += ItemStacks[i].Amount;
		ItemInfos.Add(FSigilItemInfo(ProbabilitySum, ItemStacks[i]));
	}

	if (ItemInfos.IsEmpty() || ProbabilitySum <= 0)
	{
		return Results;
	}

	const int32 RandomAmount = FMath::RandRange(MinAmount, MaxAmount);

	for (int i = 0; i < RandomAmount; i++)
	{
		auto& selectedItemInfo = GetRandomItemInfo(ItemInfos, ProbabilitySum);
		bool foundMatch = false;
		//去重，多个栈可能指向同一个道具实例，若发现重复
		for (int j = 0; j < Results.Num(); j++)
		{
			if (Results[j].Item == selectedItemInfo.Item)
			{
				Results[j] = FSigilItemInfo(Results[j].Amount + 1, Results[j]);
				foundMatch = true;
				break;
			}
		}

		if (!foundMatch) { Results.Add(FSigilItemInfo(1, selectedItemInfo)); }
	}

	return Results;
}

const FSigilItemInfo& USigilRandomItemDropperComponent::GetRandomItemInfo(const TArray<FSigilItemInfo>& ItemInfos, int32 ProbabilitySum) const
{
	const int32 RandomProbabilityIdx = FMath::RandRange(1, ProbabilitySum);

	int32 Min = 0;
	int32 Max = ItemInfos.Num() - 1;

	while (Min < Max)
	{
		const int32 Mid = Min + (Max - Min) / 2;
		if (RandomProbabilityIdx <= ItemInfos[Mid].Amount)
		{
			Max = Mid;
		}
		else
		{
			Min = Mid + 1;
		}
	}

	return ItemInfos[Min];
}
