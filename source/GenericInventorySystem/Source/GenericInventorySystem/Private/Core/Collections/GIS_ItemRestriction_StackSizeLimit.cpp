// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_ItemRestriction_StackSizeLimit.h"

#include "GIS_ItemCollection.h"
#include "GIS_ItemDefinition.h"
#include "Items/GIS_ItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemRestriction_StackSizeLimit)

UGIS_ItemRestriction_StackSizeLimit::UGIS_ItemRestriction_StackSizeLimit()
{
	DefaultStackSizeLimit = 99;
}

bool UGIS_ItemRestriction_StackSizeLimit::CanAddItemInternal_Implementation(FGIS_ItemInfo& ItemInfo, UGIS_ItemCollection* ReceivingCollection) const
{
	int32 MaxStackSize = GetStackSizeLimit(ItemInfo.Item);

	FGIS_ItemInfo ExistingItemInfoResult;
	if (!ReceivingCollection->GetItemInfo(ItemInfo.Item, ExistingItemInfoResult))
	{
		ItemInfo.Amount = FMath::Min(ItemInfo.Amount, MaxStackSize);
		return true;
	}

	int32 ItemAmountsThatFit = FMath::Min(MaxStackSize - ExistingItemInfoResult.Amount, ItemInfo.Amount);
	ItemAmountsThatFit = FMath::Max(0, ItemAmountsThatFit);

	if (ItemAmountsThatFit == 0)
	{
		return false;
	}

	ItemInfo.Amount = ItemAmountsThatFit;
	return true;
}


int32 UGIS_ItemRestriction_StackSizeLimit::GetStackSizeLimit(const UGIS_ItemInstance* Item) const
{
	if (Item == nullptr)
	{
		return 0;
	}
	if (StackSizeLimitAttributeTag.IsValid() && Item->GetDefinition()->HasIntegerAttribute(StackSizeLimitAttributeTag))
	{
		return Item->GetDefinition()->GetIntegerAttribute(StackSizeLimitAttributeTag);
	}
	// if (StackSizeLimitAttributeTag.IsValid() && Item->HasIntegerAttribute(StackSizeLimitAttributeTag))
	// {
	// 	return Item->GetIntegerAttribute(StackSizeLimitAttributeTag);
	// }

	return DefaultStackSizeLimit;
}
