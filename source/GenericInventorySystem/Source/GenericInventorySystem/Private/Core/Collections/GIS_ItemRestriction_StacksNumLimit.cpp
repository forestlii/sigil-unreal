// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_ItemRestriction_StacksNumLimit.h"

#include "GIS_InventorySystemComponent.h"
#include "GIS_ItemCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemRestriction_StacksNumLimit)

bool UGIS_ItemRestriction_StacksNumLimit::CanAddItemInternal_Implementation(FGIS_ItemInfo& ItemInfo, UGIS_ItemCollection* ReceivingCollection) const
{
	int32 ExistingNum = ReceivingCollection->GetItemStacksNum();

	int32 AvailableAdditionalStacks = MaxStacksNum - ExistingNum;

	int32 ItemAmountsThatFit = ReceivingCollection->GetItemAmountFittingInLimitedAdditionalStacks(ItemInfo, AvailableAdditionalStacks);

	if (ItemAmountsThatFit == 0)
	{
		return false;
	}

	ItemInfo = FGIS_ItemInfo(ItemAmountsThatFit, ItemInfo);
	return true;
}
