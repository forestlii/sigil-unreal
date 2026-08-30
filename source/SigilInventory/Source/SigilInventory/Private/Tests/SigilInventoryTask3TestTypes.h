// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Core/Collections/SigilItemCollection.h"
#include "Core/Collections/SigilItemRestriction.h"
#include "Crafting/SigilCraftingSystemComponent.h"
#include "Drops/SigilRandomItemDropperComponent.h"
#include "Pickups/SigilInventoryPickupComponent.h"
#include "SigilInventoryTask3TestTypes.generated.h"

UCLASS()
class USigilInventoryRemoveAmountLimitTestRestriction final : public USigilItemRestriction
{
	GENERATED_BODY()

public:
	void SetMaxRemoveAmountForTest(const int32 InMaxRemoveAmount)
	{
		MaxRemoveAmount = InMaxRemoveAmount;
	}

protected:
	virtual bool CanRemoveItemInternal_Implementation(FSigilItemInfo& ItemInfo) const override
	{
		ItemInfo.Amount = FMath::Min(ItemInfo.Amount, MaxRemoveAmount);
		return ItemInfo.Amount > 0;
	}

private:
	int32 MaxRemoveAmount = MAX_int32;
};

UCLASS()
class USigilInventoryPickupSynchronousMutationTestCollection final : public USigilItemCollection
{
	GENERATED_BODY()

public:
	void ConfigureSynchronousAddForTest(
		USigilItemInstance* InUnrelatedItem,
		const int32 InUnrelatedAmount)
	{
		UnrelatedItem = InUnrelatedItem;
		UnrelatedAmount = InUnrelatedAmount;
		bAddUnrelatedItem = true;
	}

	virtual FSigilItemInfo AddItem(const FSigilItemInfo& ItemInfo) override
	{
		const FSigilItemInfo AddedItem = Super::AddItem(ItemInfo);
		if (bAddUnrelatedItem)
		{
			bAddUnrelatedItem = false;
			Super::AddItem(FSigilItemInfo(UnrelatedItem, UnrelatedAmount));
		}
		return AddedItem;
	}

private:
	UPROPERTY()
	TObjectPtr<USigilItemInstance> UnrelatedItem;

	int32 UnrelatedAmount = 0;
	bool bAddUnrelatedItem = false;
};

UCLASS()
class USigilInventoryPickupSynchronousMutationTestCollectionDefinition final : public USigilItemCollectionDefinition
{
	GENERATED_BODY()

public:
	virtual TSubclassOf<USigilItemCollection> GetCollectionInstanceClass() const override
	{
		return USigilInventoryPickupSynchronousMutationTestCollection::StaticClass();
	}
};

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
