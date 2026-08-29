// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Crafting/SigilCraftingSystemComponent.h"
#include "Drops/SigilRandomItemDropperComponent.h"
#include "Pickups/SigilInventoryPickupComponent.h"
#include "SigilInventoryTask3TestTypes.generated.h"

UCLASS()
class USigilInventoryPickupTask3TestComponent final : public USigilInventoryPickupComponent
{
	GENERATED_BODY()

public:
	void SetInventoryForTest(USigilInventorySystemComponent* InInventory)
	{
		Inventory = InInventory;
	}

	bool AddPickupToCollectionForTest(USigilItemCollection* Destination)
	{
		return AddPickupToCollection(Destination);
	}
};

UCLASS()
class USigilCraftingTask3TestComponent final : public USigilCraftingSystemComponent
{
	GENERATED_BODY()

public:
	bool RemoveItemIngredientsForTest(
		USigilInventorySystemComponent* Inventory,
		const TArray<FSigilItemInfo>& ItemIngredients)
	{
		return RemoveItemIngredients(Inventory, ItemIngredients);
	}
};

UCLASS()
class USigilRandomItemDropperTask3TestComponent final : public USigilRandomItemDropperComponent
{
	GENERATED_BODY()

public:
	void SetDropConfigForTest(const FGameplayTag InCollectionTag, const int32 InMinAmount, const int32 InMaxAmount)
	{
		CollectionTag = InCollectionTag;
		MinAmount = InMinAmount;
		MaxAmount = InMaxAmount;
	}

	TArray<FSigilItemInfo> GetItemsToDropForTest() const
	{
		return GetItemsToDropInternal();
	}

	const FSigilItemInfo& GetRandomItemInfoForTest(
		const TArray<FSigilItemInfo>& ItemInfos,
		const int32 ProbabilitySum) const
	{
		return GetRandomItemInfo(ItemInfos, ProbabilitySum);
	}
};
