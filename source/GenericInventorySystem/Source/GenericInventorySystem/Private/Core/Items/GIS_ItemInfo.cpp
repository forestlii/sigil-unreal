// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Items/GIS_ItemInfo.h"
#include "Items/GIS_ItemInstance.h"
#include "GIS_ItemCollection.h"
#include "Items/GIS_ItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemInfo)

FGIS_ItemInfo FGIS_ItemInfo::None = FGIS_ItemInfo();

FGIS_ItemInfo::FGIS_ItemInfo()
{
	Item = nullptr;
	Amount = 0;
	ItemCollection = nullptr;
	StackId = FGIS_ItemStack::InvalidId;
	CollectionTag = FGameplayTag::EmptyTag;
	CollectionId = FGIS_ItemStack::InvalidId;
}

FGIS_ItemInfo::FGIS_ItemInfo(UGIS_ItemInstance* InItem, int32 InAmount, UGIS_ItemCollection* InCollection)
{
	Item = InItem;
	Amount = InAmount;
	ItemCollection = InCollection;
	StackId = FGIS_ItemStack::InvalidId;
}

FGIS_ItemInfo::FGIS_ItemInfo(UGIS_ItemInstance* InItem, int32 InAmount, UGIS_ItemCollection* InCollection, FGuid InStackId)
{
	Item = InItem;
	Amount = InAmount;
	ItemCollection = InCollection;
	CollectionId = InCollection->GetCollectionId();
	StackId = InStackId;
}

FGIS_ItemInfo::FGIS_ItemInfo(UGIS_ItemInstance* InItem, int32 InAmount)
{
	Item = InItem;
	Amount = InAmount;
}

FGIS_ItemInfo::FGIS_ItemInfo(UGIS_ItemInstance* InItem, int32 InAmount, FGuid InCollectionId)
{
	Item = InItem;
	Amount = InAmount;
	CollectionId = InCollectionId;
}

FGIS_ItemInfo::FGIS_ItemInfo(UGIS_ItemInstance* InItem, int32 InAmount, FGameplayTag InCollectionTag)
{
	Item = InItem;
	Amount = InAmount;
	CollectionTag = InCollectionTag;
}

FGIS_ItemInfo::FGIS_ItemInfo(int32 InAmount, const FGIS_ItemInfo& OtherInfo)
{
	Amount = InAmount;
	Item = OtherInfo.Item;
	ItemCollection = OtherInfo.ItemCollection;
	StackId = OtherInfo.StackId;
	CollectionId = OtherInfo.CollectionId;
	CollectionTag = OtherInfo.CollectionTag;
}

FGIS_ItemInfo::FGIS_ItemInfo(int32 InAmount, int32 InIndex, const FGIS_ItemInfo& OtherInfo)
{
	Amount = InAmount;
	Index = InIndex;
	Item = OtherInfo.Item;
	ItemCollection = OtherInfo.ItemCollection;
	StackId = OtherInfo.StackId;
	CollectionId = OtherInfo.CollectionId;
	CollectionTag = OtherInfo.CollectionTag;
}

FGIS_ItemInfo::FGIS_ItemInfo(UGIS_ItemInstance* InItem, int32 InAmount, const FGIS_ItemInfo& OtherInfo)
{
	Item = InItem;
	Amount = InAmount;
	ItemCollection = OtherInfo.ItemCollection;
	StackId = OtherInfo.StackId;
	CollectionId = OtherInfo.CollectionId;
}

FGIS_ItemInfo::FGIS_ItemInfo(const FGIS_ItemStack& ItemStack)
{
	Item = ItemStack.Item;
	ItemCollection = ItemStack.Collection;
	Amount = ItemStack.Amount;
	StackId = ItemStack.Id;
}

FString FGIS_ItemInfo::GetDebugString() const
{
	if (!IsValid())
	{
		return TEXT("Invalid item info");
	}

	return FString::Format(TEXT("{0}({1})"), {Item->GetDefinition()->GetName(), Amount});
}

bool FGIS_ItemInfo::operator==(const FGIS_ItemInfo& Other) const
{
	return Other.Amount == Amount && Other.Item == Item && Other.ItemCollection == ItemCollection;
}

bool FGIS_ItemInfo::IsValid() const
{
	return Item != nullptr && Amount > 0;
}

UGIS_InventorySystemComponent* FGIS_ItemInfo::GetInventory() const
{
	return ItemCollection != nullptr ? ItemCollection->GetOwningInventory() : nullptr;
}
